<?php
// $Id$

/**
 * CSA is open source Software.
 *
 * Copyright (C) 2009 SURFnet BV (http://www.surfnet.nl) and Kennisnet
 * (http://www.kennisnet.nl)
 *
 * CSA is developed for the open source Drupal platform (http://drupal.org).
 * CSA has been developed by Madcap BV (http://www.madcap.nl).
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, you can find it at:
 * http://www.gnu.org/licenses/old-licenses/gpl-2.0.html
 */

/**
 * @file
 * Profile CSA.
 */

/**
 * Return an array of the modules to be enabled when this profile is installed.
 *
 * @return
 *   An array of modules to enable.
 */
function csa_default_profile_modules() {
  return array('color', 'comment', 'help', 'menu', 'taxonomy', 'dblog', 'mediamosa_sdk', 'mediamosa_connector', 'csa', 'csa_collection', 'csa_upload', 'csa_box');
}

/**
 * Return a description of the profile for the initial installation screen.
 *
 * @return
 *   An array with keys 'name' and 'description' describing this profile,
 *   and optional 'language' to override the language selection for
 *   language-specific profiles.
 */
function csa_default_profile_details() {
  return array(
    'name' => 'Content Supplier Application default setup',
    'description' => 'To enable all features and menu\'s for CSA, use this profile. However, the setup of this profile will make your Drupal installation fixed on CSA menu items and functionality.');
}

/**
 * Return a list of tasks that this profile supports.
 *
 * @return
 *   A keyed array of tasks the profile will perform during
 *   the final stage. The keys of the array will be used internally,
 *   while the values will be displayed to the user in the installer
 *   task list.
 */
function csa_default_profile_task_list() {
}

/**
 * Perform any final installation tasks for this profile.
 *
 * The installer goes through the profile-select -> locale-select
 * -> requirements -> database -> profile-install-batch
 * -> locale-initial-batch -> configure -> locale-remaining-batch
 * -> finished -> done tasks, in this order, if you don't implement
 * this function in your profile.
 *
 * If this function is implemented, you can have any number of
 * custom tasks to perform after 'configure', implementing a state
 * machine here to walk the user through those tasks. First time,
 * this function gets called with $task set to 'profile', and you
 * can advance to further tasks by setting $task to your tasks'
 * identifiers, used as array keys in the hook_profile_task_list()
 * above. You must avoid the reserved tasks listed in
 * install_reserved_tasks(). If you implement your custom tasks,
 * this function will get called in every HTTP request (for form
 * processing, printing your information screens and so on) until
 * you advance to the 'profile-finished' task, with which you
 * hand control back to the installer. Each custom page you
 * return needs to provide a way to continue, such as a form
 * submission or a link. You should also set custom page titles.
 *
 * You should define the list of custom tasks you implement by
 * returning an array of them in hook_profile_task_list(), as these
 * show up in the list of tasks on the installer user interface.
 *
 * Remember that the user will be able to reload the pages multiple
 * times, so you might want to use variable_set() and variable_get()
 * to remember your data and control further processing, if $task
 * is insufficient. Should a profile want to display a form here,
 * it can; the form should set '#redirect' to FALSE, and rely on
 * an action in the submit handler, such as variable_set(), to
 * detect submission and proceed to further tasks. See the configuration
 * form handling code in install_tasks() for an example.
 *
 * Important: Any temporary variables should be removed using
 * variable_del() before advancing to the 'profile-finished' phase.
 *
 * @param $task
 *   The current $task of the install system. When hook_profile_tasks()
 *   is first called, this is 'profile'.
 * @param $url
 *   Complete URL to be used for a link or form action on a custom page,
 *   if providing any, to allow the user to proceed with the installation.
 *
 * @return
 *   An optional HTML string to display to the user. Only used if you
 *   modify the $task, otherwise discarded.
 */
function csa_default_profile_tasks(&$task, $url) {

  // Insert default user-defined node types into the database. For a complete
  // list of available node type attributes, refer to the node type API
  // documentation at: http://api.drupal.org/api/HEAD/function/hook_node_info.
  $types = array(
    array(
      'type' => 'page',
      'name' => st('Page'),
      'module' => 'node',
      'description' => st("A <em>page</em>, similar in form to a <em>story</em>, is a simple method for creating and displaying information that rarely changes, such as an \"About us\" section of a website. By default, a <em>page</em> entry does not allow visitor comments and is not featured on the site's initial home page."),
      'custom' => TRUE,
      'modified' => TRUE,
      'locked' => FALSE,
      'help' => '',
      'min_word_count' => '',
    ),
    array(
      'type' => 'story',
      'name' => st('Story'),
      'module' => 'node',
      'description' => st("A <em>story</em>, similar in form to a <em>page</em>, is ideal for creating and displaying content that informs or engages website visitors. Press releases, site announcements, and informal blog-like entries may all be created with a <em>story</em> entry. By default, a <em>story</em> entry is automatically featured on the site's initial home page, and provides the ability to post comments."),
      'custom' => TRUE,
      'modified' => TRUE,
      'locked' => FALSE,
      'help' => '',
      'min_word_count' => '',
    ),
  );

  foreach ($types as $type) {
    $type = (object) _node_type_set_defaults($type);
    node_type_save($type);
  }

  // Default page to not be promoted and have comments disabled.
  variable_set('node_options_page', array('status'));
  variable_set('comment_page', COMMENT_NODE_DISABLED);

  // Settings.
  $theme_settings = array(
    'toggle_logo' => 0,
    'toggle_name' => 1,
    'toggle_slogan' => 0,
    'toggle_mission' => 1,
    'toggle_node_user_picture' => 0,
    'toggle_comment_user_picture' => 0,
    'toggle_search' => 0,
    'toggle_favicon' => 1,
    'toggle_primary_links' => 1,
    'toggle_secondary_links' => 0,
    'toggle_node_info_page' => 0,
    'toggle_node_info_story' => 1,
    'default_logo' => 1,
    'logo_path' => '',
    'logo_upload' => '',
    'default_favicon' => 1,
    'favicon_path' => '',
    'favicon_upload' => '',
  );
  variable_set('theme_settings', $theme_settings);

  $theme_csa_base_settings = array(
    'csa_base_move_sidebar' => 1,
    'breadcrumb_display' => 0,
    'breadcrumb_display_admin' => 1,
    'breadcrumb_with_title' => 0,
    'primary_links_display_style' => 'tabbed-menu',
    'primary_links_allow_tree' => 0,
    'secondary_links_display_style' => 'menu',
    'secondary_links_allow_tree' => 0,
    'search_snippet' => 1,
    'search_info_type' => 1,
    'search_info_user' => 1,
    'search_info_date' => 1,
    'search_info_comment' => 1,
    'search_info_upload' => 1,
    'mission_statement_pages' => 'home',
    'hide_front_page_title' => 1,
    'front_page_title_display' => 'title_slogan',
    'page_title_display_custom' => '',
    'other_page_title_display' => 'ptitle_slogan',
    'other_page_title_display_custom' => '',
    'configurable_separator' => ' | ',
    'meta_keywords' => '',
    'meta_description' => '',
    'taxonomy_display_default' => 'only',
    'taxonomy_display_vocab_name' => 1,
    'taxonomy_format_default' => 'vocab',
    'taxonomy_format_links' => 0,
    'taxonomy_format_delimiter' => ', ',
    'taxonomy_enable_content_type' => 0,
    'submitted_by_author_default' => 0,
    'submitted_by_date_default' => 0,
    'submitted_by_enable_content_type' => 0,
    'mission' => '',
    'default_logo' => 1,
    'logo_path' => '',
    'default_favicon' => 1,
    'favicon_path' => '',
    'primary_links' => 1,
    'secondary_links' => 1,
    'toggle_logo' => 0,
    'toggle_favicon' => 1,
    'toggle_name' => 1,
    'toggle_search' => 0,
    'toggle_slogan' => 0,
    'toggle_mission' => 1,
    'toggle_node_user_picture' => 0,
    'toggle_comment_user_picture' => 0,
    'toggle_primary_links' => 1,
    'toggle_secondary_links' => 1,
    'toggle_node_info_page' => 0,
    'toggle_node_info_story' => 1,
    'logo_upload' => '',
    'favicon_upload' => '',
    'taxonomy_display_page' => 'only',
    'taxonomy_format_page' => 'vocab',
    'submitted_by_author_page' => 1,
    'submitted_by_date_page' => 1,
    'taxonomy_display_story' => 'only',
    'taxonomy_format_story' => 'vocab',
    'submitted_by_author_story' => 1,
    'submitted_by_date_story' => 1,
  );
  variable_set('theme_csa_base_settings', $theme_csa_base_settings);

  // Create basic roles.
  // Make an 'CSA-Admin' role.
  db_query("INSERT INTO {role} (rid, name) VALUES (3, 'CSA Admin')");
  db_query("INSERT INTO {role} (rid, name) VALUES (4, 'CSA User')");

  // Permissions.
  db_query("INSERT INTO {permission} (rid, perm, tid) VALUES(3, 'administer csa settings, administer csa user settings, administer mediamosa connector settings', 0)");
  db_query("INSERT INTO {permission} (rid, perm, tid) VALUES(4, 'csa user, csa upload mediafile', 0)");

  // Administrator also has CSA Admin role so he can see the navigation block.
  db_query("INSERT INTO {users_roles} (uid, rid) VALUES (1, 3)");

  // Block navigation linked to CSA admin role.
  db_query("INSERT INTO {blocks_roles} (module, delta, rid) VALUES ('user', 1, 3)");

  // Set blocks.
  db_query("INSERT INTO {blocks} SET module = 'csa', delta = 'csa-version', theme = 'csa_base', status = 1, region = 'footer', pages = ''");
  db_query("INSERT INTO {blocks} SET module = 'csa', delta = 'csa-status-bar', theme = 'csa_base', status = 1, region = 'header', pages = ''");
  db_query("INSERT INTO {blocks} SET module = 'csa', delta = 'csa-previous-next-top', theme = 'csa_base', status = 1, region = 'content_top', pages = ''");
  db_query("INSERT INTO {blocks} SET module = 'csa', delta = 'csa-previous-next-bottom', theme = 'csa_base', status = 1, region = 'content_bottom', pages = ''");
  db_query("INSERT INTO {blocks} SET module = 'user', delta = '0', theme = 'csa_base', status = 1, region = 'sidebar_1', pages = ''");
  db_query("INSERT INTO {blocks} SET module = 'user', delta = '1', theme = 'csa_base', status = 1, region = 'sidebar_1', pages = ''");

  // Create Home Page.
  $node = new StdClass();
  $node->type = 'page';
  $node->status = 1;
  $node->promote = 0;
  $node->uid = 1;
  $node->name = 'admin';
  $node->path = 'home';
  $node->title = 'Welcome to CSA';
  $node->body = st('Lorem ipsum dolor sit amet, consectetuer adipiscing elit, sed diam nonummy nibh euismod tincidunt.');
  node_save($node);
  variable_set('site_frontpage', 'node/1');

  // Turn on user pictures
  variable_set('user_pictures', 1);
  variable_set('user_picture_path', 'pictures');

  variable_set('user_picture_dimensions', '500x300');
  variable_set('user_picture_file_size', '2048'); // 2mb max.


  // Configure user settings. Set user creation to administrator only.
  variable_set('user_register', '0');

  // Now picture support is enabled, check whether the picture directory exists:
  $picture_path = file_create_path(variable_get('user_picture_path', 'pictures'));
  file_check_directory($picture_path, 1, 'user_picture_path');

  // Primary menu items.
  $item = array(
    'link_path' => '<front>',
    'router_path' => '',
    'link_title' => 'Home',
    'weight' => -50,
    'menu_name' => 'primary-links',
    'expanded' => 0,
    'options' => array(
      'attributes' => array(
        'title' => '',
       ),
     ),
  );
  menu_link_save($item);

  $item = array(
    'link_path' => 'admin/mediamosa/config/connector',
    'router_path' => 'admin/mediamosa/config/connector',
    'link_title' => 'MediaMosa Connector',
    'weight' => -45,
    'menu_name' => 'primary-links',
    'expanded' => 0,
    'options' => array(
      'attributes' => array(
        'title' => '',
       ),
     ),
  );
  menu_link_save($item);
  $item = array(
    'link_path' => 'csa/upload',
    'router_path' => 'csa/upload',
    'link_title' => 'Upload',
    'weight' => -40,
    'menu_name' => 'primary-links',
    'expanded' => 0,
    'options' => array(
      'attributes' => array(
        'title' => '',
       ),
     ),
  );
  menu_link_save($item);

  $item = array(
    'link_path' => 'csa/unpublished',
    'router_path' => 'csa/unpublished',
    'link_title' => 'Unpublished',
    'weight' => -35,
    'menu_name' => 'primary-links',
    'expanded' => 0,
    'options' => array(
      'attributes' => array(
        'title' => '',
       ),
     ),
  );
  menu_link_save($item);

  $item = array(
    'link_path' => 'csa/published',
    'router_path' => 'csa/published',
    'link_title' => 'Published',
    'weight' => -30,
    'menu_name' => 'primary-links',
    'expanded' => 0,
    'options' => array(
      'attributes' => array(
        'title' => '',
       ),
     ),
  );
  menu_link_save($item);

  $item = array(
    'link_path' => 'csa/search',
    'router_path' => 'csa/search',
    'link_title' => 'Search',
    'weight' => -25,
    'menu_name' => 'primary-links',
    'expanded' => 0,
    'options' => array(
      'attributes' => array(
        'title' => '',
       ),
     ),
  );
  menu_link_save($item);

  $item = array(
    'link_path' => 'csa/collection',
    'router_path' => 'csa/collection',
    'link_title' => 'Collections',
    'weight' => -20,
    'menu_name' => 'primary-links',
    'expanded' => 0,
    'options' => array(
      'attributes' => array(
        'title' => '',
       ),
     ),
  );
  menu_link_save($item);

  $item = array(
    'link_path' => 'csa/prefs',
    'router_path' => 'csa/prefs',
    'link_title' => 'Prefs',
    'weight' => -15,
    'menu_name' => 'primary-links',
    'expanded' => 0,
    'options' => array(
      'attributes' => array(
        'title' => '',
       ),
     ),
  );
  menu_link_save($item);

  $item = array(
    'link_path' => 'csa/settings',
    'router_path' => 'csa/settings',
    'link_title' => 'CSA Settings',
    'weight' => -10,
    'menu_name' => 'primary-links',
    'expanded' => 0,
    'options' => array(
      'attributes' => array(
        'title' => '',
       ),
     ),
  );
  menu_link_save($item);

  // Make CSA menu hooks act on the primary block.
  variable_set('csa_has_primary_block', TRUE);

  // All off.
  db_query("UPDATE {system} SET status = 0 WHERE type = 'theme'");
  // Set CSA base as default theme.
  db_query("UPDATE {system} SET status = 1 WHERE type = 'theme' AND name = 'csa_base'");
  variable_set('theme_default', 'csa_base');

  // Rebuild.
  list_themes(TRUE);
  menu_rebuild();
  drupal_rebuild_theme_registry();
}

/**
 * Implementation of hook_form_alter().
 *
 * Allows the profile to alter the site-configuration form. This is
 * called through custom invocation, so $form_state is not populated.
 */
function csa_default_form_alter(&$form, $form_state, $form_id) {
  if ($form_id == 'install_configure') {
    // Set default for site name field.
    $form['site_information']['site_name']['#default_value'] = 'Content Supplier Application';
  }
}
