<?php
// $Id$

/**
 * @file template.php
 * Modifies and adds theme functions for use in the csa_base.
 *
 * This is the Madcap base theme. It is meant as a good, complete base for developing
 * client specific themes. Therefore it has very little styling / graphics. The focus is
 * mainly on having a good CSS base for further development, and on having some base features
 * in the theme. The theme should also provide a lot of basic styling for standard Drupal
 * functions, the admin pages and some other usablity improvements.
 */

/**
 * Load default theme settings
 */
include_once dirname(__FILE__) . '/theme-settings-defaults.php';
csa_base_get_default_settings($GLOBALS['theme_key']);

/**
 * Implementation of theme_admin_block().
 *
 * Admin Block Theme: Provide useful icons in the 'Administer'-section.
 */
function csa_base_admin_block($block) {
  global $zebra;
  // Don't display the block if it has no content to display.
  if (empty($block['content'])) {
    return '';
  }

  // Create a class for the block's title H3 without HTML, lowercase, no spaces. Classname is based on path.
  $blockclass = 'admin-panel-'. csa_base_safe_css_name(str_replace('admin/', '', $block['path']));
  $output = <<< EOT
  <div class="admin-panel $blockclass">
    <h3>$block[title]</h3>
    <div class="body">
      <p class="description">
        $block[description]
      </p>
      $block[content]
    </div>
  </div>
EOT;
  return $output;
}

/**
 * Implementation of theme_blocks().
 *
 * Print dividers after each second, third or fourth block. These can be used
 * to 'clear' them. This makes it easy te make rows of two, three or four blocks.
 */
function csa_base_blocks($region) {
  $output = '';
  $block_list = block_list($region);
  $block_count = count($block_list);

  if ($block_count) {
    $i = 1;
    foreach ($block_list as $key => $block) {
      $classes = array();
      $output .= theme('block', $block);

      if ($i % 5 == 0) $classes[] = 'after-fifth-block';
      if ($i % 4 == 0) $classes[] = 'after-fourth-block';
      if ($i % 3 == 0) $classes[] = 'after-third-block';
      if ($i % 2 == 0) $classes[] = 'after-second-block';

      if ($i == $block_count) {
        $classes[] = 'after-last-block';
      }
      if (count($classes) > 0) {
        $output .= '<div class="'. implode(' ', $classes) .'"></div>' ."\n";
      }
      $i++;
    }
  }

  // Add any content assigned to this region through drupal_set_content() calls.
  $output .= drupal_get_content($region);
  return $output;
}

/**
 * Implementation of theme_breadcrumb().
 *
 * Allow the title to be parsed in the breadcrumb &
 * unset the breadcrumb on the frontpage.
 */
function csa_base_breadcrumb($breadcrumb, $title = NULL) {
  if (drupal_is_front_page()) {
    $output = '';
  }
  elseif (!empty($breadcrumb) && (count($breadcrumb)>0) || !empty($title)) {
    $output = implode(' » ', $breadcrumb);
    if (!empty($title)) {
      $output .=  ' » <span class="title">'. $title .'</span>';
    }
    $output = '<div class="breadcrumb">'. $output .'</div>';
  }

  return $output;
}

/**
 * Implementation of theme_button().
 *
 * Add span around submit inputs so they can be styled cross-browser.
 * The button-inner wrapper is needed for Internet Explorer's margin inheritance bug.
 */
function csa_base_button($element) {
  return '<span class="button form-'. $element['#button_type'] .'"><span class="button-inner"><input type="submit" '. (empty($element['#name']) ? '' : 'name="'. $element['#name'] .'" ') .'id="'. $element['#id'] .'"'. drupal_attributes($element['#attributes']) .' value="'. check_plain($element['#value']) .'" /></span></span>';
}

/**
 * Implementation of theme_help().
 */
function csa_base_help() {
  if ($help = menu_get_active_help()) {
    return '<div class="help"><div class="messages-inner">'. $help .'</div></div>';
  }
}

/**
 * Implementation of theme_menu_item_link().
 *
 * Add an extra span inside menu links.
 */
function csa_base_menu_item_link($link) {
  if (empty($link['localized_options'])) {
    $link['localized_options'] = array();
  }
  $link['localized_options']['html'] = TRUE;

  if (!empty($link['in_active_trail'])) {
    $link['localized_options']['attributes']['class'] = 'active-trail';
  }
  return l('<span>'. $link['title'] .'</span>', $link['href'], $link['localized_options']);
}

/**
 * Implementation of theme_menu_local_tasks().
 */
function csa_base_menu_local_tasks() {
  $output = '';

  if ($primary = menu_primary_local_tasks()) {
    $output .= "<ul class=\"tabs primary\">\n". $primary ."</ul>\n";
  }

  if ($secondary = menu_secondary_local_tasks()) {
    $output .= "</div>\n<div class=\"tabs secondary\">";
    $output .= "<ul class=\"tabs secondary\">\n". $secondary ."</ul>\n";
  }

  return $output;
}

/**
 * Implementation of template_preprocess_block().
 */
function csa_base_preprocess_block(&$variables) {
  // add meaningfull classes to the block...
  $block_classes = array();
  $block_classes[] = 'block';
  $block_classes[] = 'block-'. $variables['block']->module;

  if (!empty($variables['skinr'])) {
    $block_classes[] = $variables['skinr'];
  }

  $variables['block_classes'] = implode(' ', $block_classes);
}

/**
 * Implementation of template_preprocess_comment().
 */
function csa_base_preprocess_comment(&$variables) {
  $comment_classes = array();
  $comment_classes[] = 'comment';

  if ($variables['comment']->status == COMMENT_NOT_PUBLISHED) {
    $comment_classes[] = 'comment-unpublished';
  }

  $comments_per_page = _comment_get_display_setting('comments_per_page', $variables['node']);
  $comments_left = $variables['node']->comment_count % $comments_per_page;
  if($variables['id'] == $comments_per_page || $variables['id'] == $comments_left ) {
    $comment_classes[] = 'comment-last';
  }
  elseif ($variables['id'] == 1) {
    $comment_classes[] = 'comment-first';
  }

  $variables['comment_classes'] = implode(' ', $comment_classes);
}

/**
 * Implementation of template_preprocess_maintenance_page().
 */
function csa_base_preprocess_maintenance_page(&$vars) {
  csa_base_preprocess_page($vars);
}

/**
 * Implementation of template_preprocess_node().
 */
function csa_base_preprocess_node(&$variables) {
  // Add meaningfull classes to the node.
  $node_classes = array();
  $node_classes[] = 'node';
  $node_classes[] = 'node-'. $variables['node']->type;
  if (empty($variables['status'])) {
    $node_classes[] = 'node-unpublished';
  }
  $variables['node_classes'] = implode(' ', $node_classes);

  // Date & author.
  $date = t('Posted ') . format_date($variables['node']->created, 'medium'); // Format date as small, medium, or large
  $author = theme('username', $variables['node']);
  $author_only_separator = t('Posted by ');
  $author_date_separator = t(' by ');
  $submitted_by_content_type = (theme_get_setting('submitted_by_enable_content_type') == 1) ? $variables['node']->type : 'default';
  $date_setting = (theme_get_setting('submitted_by_date_'. $submitted_by_content_type) == 1);
  $author_setting = (theme_get_setting('submitted_by_author_'. $submitted_by_content_type) == 1);
  $author_separator = ($date_setting) ? $author_date_separator : $author_only_separator;
  $date_author = ($date_setting) ? $date : '';
  $date_author .= ($author_setting) ? $author_separator . $author : '';
  $variables['submitted'] = $date_author;

  if (theme_get_setting('hide_front_page_title') && drupal_is_front_page()) {
    $variables['submitted'] = NULL;
  }

  $taxonomy_content_type  = theme_get_setting('taxonomy_enable_content_type') == 1 ? $variables['node']->type : 'default';
  $taxonomy_display       = theme_get_setting('taxonomy_display_'. $taxonomy_content_type);
  $taxonomy_format        = theme_get_setting('taxonomy_format_'. $taxonomy_content_type);
  $taxonomy_format_links  = theme_get_setting('taxonomy_format_links');

  if (module_exists('taxonomy') && ($taxonomy_display == 'all' || ($taxonomy_display == 'only' && $variables['page']))) {
    $output       = array();
    $vocabularies = taxonomy_get_vocabularies($variables['node']->type);
    foreach ($vocabularies as $vocabulary) {
      if (theme_get_setting('taxonomy_vocab_display_'. $taxonomy_content_type .'_'. $vocabulary->vid)) {
        $terms = taxonomy_node_get_terms_by_vocabulary($variables['node'], $vocabulary->vid);
        if ($terms) {
          $links = array();
          foreach ($terms as $term) {
            $links[] = array(
              'data'  => l($term->name, taxonomy_term_path($term), array('attributes' => array('rel' => 'tag', 'title' => strip_tags($term->description)))),
              'class' => 'vocab-term',
            );
          }
          if ($taxonomy_format == 'vocab') {
            $data     = theme_get_setting('taxonomy_display_vocab_name') ? '<span class="vocab-name">'. $vocabulary->name .':</span> ' : '';
            $class    = array('vocab');
            $children = array();
            if ($taxonomy_format_links) {
              $class[] = 'links';
              $children = $links;
            }
            else {
              $term_list = array();
              foreach($links as $link) {
                $term_list[] = $link['data'];
              }
              $data .= implode(theme_get_setting('taxonomy_format_delimiter'), $term_list);
            }
            $output[] = array(
              'data'      => $data,
              'class'     => implode(' ', $class),
              'id'        => 'vocab-'. $vocabulary->vid,
              'children'  => $children,
            );
          }
          else {
            $output = array_merge($links, $output);
          }
        }
      }
    }
    $variables['terms'] = theme('item_list', $output, null, 'ul', array('class' => 'taxonomy'));
  }
  else {
    $variables['terms'] = '';
  }
}

/**
 * Implementation of template_preprocess_page().
 */
function csa_base_preprocess_page(&$variables) {
  $conditional  = array();
  $query_string = '?'. substr(variable_get('css_js_query_string', '0'), 0, 1);

  $conditional['IE']    = array(); // Target all IE versions
  $conditional['IE 6']  = array(); // Target Internet Explorer 6 only
  $conditional['IE 7']  = array(); // Target Internet Explorer 7 only
  $conditional['IE 8']  = array(); // Target Internet Explorer 8 only

  $conditional['IE 6'][] .= '<script type="text/javascript">var blankImgIE="'. theme('theme_path', '/images/blank.gif') .'";</script>';
  $conditional['IE 6'][] .= '<style type="text/css" media="all">@import "'. theme('theme_path', '/css/fix-ie-6.css') . $query_string .'";</style>';
  $conditional['IE 6'][] .= '<style type="text/css">img { behavior: url('. theme('theme_path', '/script/iepngfix.htc') . $query_string .') }</style>';
  $conditional['IE 7'][] .= '<style type="text/css" media="all">@import "'. theme('theme_path', '/css/fix-ie-7.css') . $query_string .'";</style>';

  $conditional_output = '';
  foreach ($conditional as $version => $rules) {
    if (count($rules)) {
      $conditional_output .= '<!--[if '. $version ."]>\n";
      foreach ($rules as $rule) {
        $conditional_output .= $rule ."\n";
      }
      $conditional_output .= "<![endif]-->\n";
    }
  }

  // Rebuild the $scripts output
  $js = drupal_add_js();

  // remove sticky table headers, we use our own modified version for this
  unset($js['module']['misc/tableheader.js']);
  $variables['scripts'] = drupal_get_js('header', $js) . $conditional_output;

  // Rebuild the $styles output
  $css = drupal_add_css();
  $variables['styles'] = drupal_get_css($css);
  $http = empty($_SERVER['HTTPS']) ? 'http' : 'https';

  $variables['styles'] .= "<link href='" . $http . "://fonts.googleapis.com/css?family=Molengo' rel='stylesheet' type='text/css'>\n";
  $variables['styles'] .= "<link href='" . $http . "://fonts.googleapis.com/css?family=Droid+Sans' rel='stylesheet' type='text/css'>\n";

  // add a var $admin_section to see if we are in the admin section of the site
  $variables['admin_section'] = FALSE;
  if (arg(0) == 'admin' || arg(2) == 'edit' || arg(2) == 'webform-results') {
    $variables['body_classes'] .= ' admin-section';
    $variables['admin_section'] = TRUE;
  }

  // Move second sidebar to first if this option is enabled in the theme settings and the user is viewing an admin page
  if (theme_get_setting('csa_base_move_sidebar') && $variables['admin_section']) {
    if (!empty($variables['sidebar_1']) && !empty($variables['sidebar_2'])) {
      $variables['sidebar_1'] .= $variables['sidebar_2'];
      unset($variables['sidebar_2']);
    }
    elseif (!empty($variables['sidebar_2'])) {
      $variables['sidebar_1'] = $variables['sidebar_2'];
      unset($variables['sidebar_2']);
    }
  }

  // Set up layout variable
  $variables['layout'] = 'none';
  if (!empty($variables['sidebar_1'])) {
    $variables['layout'] = 'sidebar-1';
  }
  if (!empty($variables['sidebar_2'])) {
    $variables['layout'] = ($variables['layout'] == 'sidebar-1') ? 'both' : 'sidebar-2';
  }

  // Strip sidebar classes from the body
  $variables['body_classes'] = str_replace(array('both', 'no-sidebars', 'two-sidebars', 'one-sidebar', 'sidebar-left', 'sidebar-right'), '',  $variables['body_classes']);

  // Remove excess spaces
  $variables['body_classes'] = str_replace('  ', ' ', trim($variables['body_classes']));

  // Add information about the number of sidebars
  if ($variables['layout'] == 'both') {
    $variables['body_classes'] .= ' two-sidebars';
  }
  elseif ($variables['layout'] == 'none') {
    $variables['body_classes'] .= ' no-sidebars';
  }
  else {
    $variables['body_classes'] .= ' one-sidebar '. $variables['layout'];
  }

  // add the taxonomy terms to the body_classes
  if (module_exists('taxonomy') && !empty($variables['node_terms'])) {
    $terms = array();
    foreach (taxonomy_node_get_terms($variables['node']) as $term) {
      $terms[] = $variables['node_terms'] . csa_base_safe_css_name($term->name);
    }
    if (count($terms)) {
      $variables['body_classes'] .= ' '. implode(' ', $terms);
    }
  }
  if (!empty($variables['logo'])){
    $logo_img = theme('image', substr($variables['logo'], strlen(base_path()), strlen($variables['logo'])), $variables['site_name'], $variables['site_name']);
    $variables['logo'] = l($logo_img, "<front>", array('html' => 'true', 'attributes' => array('title' => $variables['site_name'])));
  }
  // Display mission statement on all pages?
  if (theme_get_setting('mission_statement_pages') == 'all') {
    $variables['mission'] = theme_get_setting('mission', FALSE);
  }

  // Show the title in the breadcrumb?
  if ((!theme_get_setting('breadcrumb_display_admin') && $variables['admin_section']) || (theme_get_setting('breadcrumb_display') == 0 && !$variables['admin_section'])) { //Hide breadcrumb on all pages?
    unset($variables['breadcrumb']);
  }
  elseif (theme_get_setting('breadcrumb_with_title')) {
    $variables['breadcrumb'] = theme('breadcrumb', drupal_get_breadcrumb(), $variables['title']);
  }

  $title = t(variable_get('site_name', ''));
  $slogan = t(variable_get('site_slogan', ''));
  $mission = t(variable_get('site_mission', ''));
  $page_title = t(drupal_get_title());
  $title_separator = theme_get_setting('configurable_separator');

  // Front page title settings
  if (drupal_is_front_page()) {
    switch (theme_get_setting('front_page_title_display')) {
      case 'title_slogan':
        $variables['head_title'] = drupal_set_title($title . $title_separator . $slogan);
        break;
      case 'slogan_title':
        $variables['head_title'] = drupal_set_title($slogan . $title_separator . $title);
        break;
      case 'title_mission':
        $variables['head_title'] = drupal_set_title($title . $title_separator . $mission);
        break;
      case 'custom':
        if (theme_get_setting('page_title_display_custom') !== '') {
          $variabless['head_title'] = drupal_set_title(t(theme_get_setting('page_title_display_custom')));
        }
    }
  }
  else { // Non-front page title settings
    switch (theme_get_setting('other_page_title_display')) {
      case 'ptitle_slogan':
        $variables['head_title'] = drupal_set_title($page_title . $title_separator . $slogan);
        break;
      case 'ptitle_stitle':
        $variables['head_title'] = drupal_set_title($page_title . $title_separator . $title);
        break;
      case 'ptitle_smission':
        $variables['head_title'] = drupal_set_title($page_title . $title_separator . $mission);
        break;
      case 'ptitle_custom':
        if (theme_get_setting('other_page_title_display_custom') !== '') {
          $variables['head_title'] = drupal_set_title($page_title . $title_separator . t(theme_get_setting('other_page_title_display_custom')));
        }
        break;
      case 'custom':
        if (theme_get_setting('other_page_title_display_custom') !== '') {
          $variables['head_title'] = drupal_set_title(t(theme_get_setting('other_page_title_display_custom')));
        }
    }
  }

  // Set variables for the primary and secondary links
  if (!empty($variables['primary_links'])) {
    if (theme_get_setting('primary_links_allow_tree')) {
      $variables['primary_menu'] = menu_tree(variable_get('menu_primary_links_source', 'primary-links'));
    }
    else {
      $variables['primary_menu'] = theme('links', $variables['primary_links'], array('class' => 'menu primary-links'));
    }
  }
  if (!empty($variables['secondary_links'])) {
    if (theme_get_setting('secondary_links_allow_tree')) {
      $variables['secondary_menu'] = menu_tree(variable_get('menu_secondary_links_source', 'secondary-links'));
    }
    else {
      $variables['secondary_menu'] = theme('links', $variables['secondary_links'], array('class' => 'menu secondary-links'));
    }
  }

  if (theme_get_setting('hide_front_page_title') && drupal_is_front_page()) {
    $variables['title'] = NULL;
  }
  else {
    // Remove any potential html tags
    $variables['head_title'] = strip_tags($variables['head_title']);
  }
}

/**
 * Implementation of template_preprocess_search_result().
 *
 * Modify search results based on theme settings,
 * Borrowed from Acquia_marina.
 */
function csa_base_preprocess_search_result(&$variables) {
  static $search_zebra = 'even';
  $search_zebra = ($search_zebra == 'even') ? 'odd' : 'even';
  $variables['search_zebra'] = $search_zebra;

  $result = $variables['result'];
  $variables['url'] = check_url($result['link']);
  $variables['title'] = check_plain($result['title']);

  // Check for existence. User search does not include snippets.
  $variables['snippet'] = '';
  if (isset($result['snippet']) && theme_get_setting('search_snippet')) {
    $variables['snippet'] = $result['snippet'];
  }

  $info = array();
  if (!empty($result['type']) && theme_get_setting('search_info_type')) {
    $info['type'] = check_plain($result['type']);
  }

  if (!empty($result['user']) && theme_get_setting('search_info_user')) {
    $info['user'] = $result['user'];
  }

  if (!empty($result['date']) && theme_get_setting('search_info_date')) {
    $info['date'] = format_date($result['date'], 'small');
  }

  if (isset($result['extra']) && is_array($result['extra'])) {
    // $info = array_merge($info, $result['extra']);  Drupal bug?  [extra] array not keyed with 'comment' & 'upload'
    if (!empty($result['extra'][0]) && theme_get_setting('search_info_comment')) {
      $info['comment'] = $result['extra'][0];
    }

    if (!empty($result['extra'][1]) && theme_get_setting('search_info_upload')) {
      $info['upload'] = $result['extra'][1];
    }
  }

  // Provide separated and grouped meta information.
  $variables['info_split'] = $info;
  $variables['info'] = implode(' - ', $info);

  // Provide alternate search result template.
  $variables['template_files'][] = 'search-result-'. $variables['type'];
}

/**
 * Implementation of template_preprocess_views_view_grid().
 *
 * Rewrite the output of $vars['rows'] for vertical aligned grids
 * so they can be used in csa_base's implementation of the view layout.
 */
function csa_base_preprocess_views_view_grid(&$vars) {
  $view     = $vars['view'];
  $result   = $view->result;
  $options  = $view->style_plugin->options;
  $handler  = $view->style_plugin;

  if ($options['alignment'] == 'vertical') {
    $rows = array();
    $count_rows = 0;
    $count_cols = 0;

    while ($count_rows <= count($vars['rows'])) {
      foreach ($vars['rows'] as $count => $item) {
        if ($count_cols == 3) {
          $count_rows++;
          $count_cols = 0;
        }
        if (!$item) {
          unset($vars['rows'][$count]);
        }
        else {
          $rows[$count_rows][] = array_shift($vars['rows'][$count]);
          $count_cols++;
        }
      }
    }

    $vars['rows'] = $rows;
  }
}

/**
 * Implementation of theme_status_messages().
 */
function csa_base_status_messages($display = NULL) {
  $output = '';
  foreach (drupal_get_messages($display) as $type => $messages) {
    $output .= "<div class=\"messages $type\">\n<div class=\"messages-inner\">\n";
    if (count($messages) > 1) {
      $output .= " <ul>\n";
      foreach ($messages as $message) {
        $output .= '  <li>'. $message ."</li>\n";
      }
      $output .= " </ul>\n";
    }
    else {
      $output .= $messages[0];
    }
    $output .= "</div>\n</div>\n";
  }
  return $output;
}

/**
 * Convert a string to a name safe for use as a css class-name.
 */
function csa_base_safe_css_name($string) {
  if (is_numeric($string{0})) {
    // if the first character is numeric, put 'n' in front
    $string = 'n'. $string;
  }
  return drupal_strtolower(preg_replace('/[^a-zA-Z0-9-]+/', '-', $string));
}

/**
 * Implementation of hook_theme().
 */
function csa_base_theme($existing, $type, $theme, $path) {
  return array(
    'safe_css_name' => array(
      'arguments' => array('display' => NULL),
    ),
    'theme_path' => array(
      'arguments' => array('path' => NULL),
    ),
  );
}

/**
 * Check the path to a file in the theme directory
 */
function csa_base_theme_path($path) {
  global $theme;

  if (file_exists(drupal_get_path('theme', $theme) . $path)) {
    return drupal_get_path('theme', $theme) . $path;
  }
  elseif (file_exists(path_to_theme() . $path)) {
    return path_to_theme() . $path;
  }
  elseif (file_exists(drupal_get_path('theme', 'csa_base') . $path)) {
    // this is really just a fallback
    return drupal_get_path('theme', 'csa_base') . $path;
  }

  return FALSE;
}
/*
 * Modded output for additional span's, so that the primary
 * links can be styled as tabs
 */
function csa_base_links($links, $attributes = array('class' => 'links')) {
  global $language;
  $output = '';

  if (count($links) > 0) {
    $output = '<ul'. drupal_attributes($attributes) .'>';

    $num_links = count($links);
    $i = 1;

    foreach ($links as $key => $link) {
      $class = $key;

      // Add first, last and active classes to the list of links to help out themers.
      if ($i == 1) {
        $class .= ' first';
      }
      if ($i == $num_links) {
        $class .= ' last';
      }
      if (isset($link['href']) && ($link['href'] == $_GET['q'] || ($link['href'] == '<front>' && drupal_is_front_page()))
          && (empty($link['language']) || $link['language']->language == $language->language)) {
        $class .= ' active';
      }
      $output .= '<li'. drupal_attributes(array('class' => $class)) .'>';

      if (isset($link['href'])) {
        // Pass in $link as $options, they share the same keys.
        // modified to allow <span>'s in the output...
        $link['html'] = true;
        $output .= l('<span>' . $link['title'] . '</span>', $link['href'], $link);
      }
      else if (!empty($link['title'])) {
        // Some links are actually not links, but we wrap these in <span> for adding title and class attributes
        if (empty($link['html'])) {
          $link['title'] = check_plain($link['title']);
        }
        $span_attributes = '';
        if (isset($link['attributes'])) {
          $span_attributes = drupal_attributes($link['attributes']);
        }
        $output .= '<span'. $span_attributes .'>'. $link['title'] .'</span>';
      }

      $i++;
      $output .= "</li>\n";
    }

    $output .= '</ul>';
  }

  return $output;
}
