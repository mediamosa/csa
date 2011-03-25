<?php
// $Id$

/**
 * @file theme-settings.php
 * Sets the values for various theme settings within the csa_base.
 */
include_once './' . drupal_get_path('theme', 'csa_base') . '/theme-settings-defaults.php';

function csa_base_settings($saved_settings, $subtheme_defaults = array()) {
  // Get the node types
  $node_types = node_get_types('names');

  // Get the default values from the .info file.
  $defaults = csa_base_get_default_settings('csa_base');

  // Allow a subtheme to override the default values.
  $defaults = array_merge($defaults, $subtheme_defaults);

  // Set the default values for content-type-specific settings
  foreach ($node_types as $type => $name) {
    $defaults["taxonomy_display_{$type}"]         = $defaults['taxonomy_display_default'];
    $defaults["taxonomy_format_{$type}"]          = $defaults['taxonomy_format_default'];
    $defaults["submitted_by_author_{$type}"]      = $defaults['submitted_by_author_default'];
    $defaults["submitted_by_date_{$type}"]        = $defaults['submitted_by_date_default'];
  }

  // Merge the saved variables and their default values
  $settings = array_merge($defaults, $saved_settings);

  // Admin settings
  $form['admin_settings'] = array(
    '#type' => 'fieldset',
    '#title' => t('Admin'),
    '#description' => t('Control how csa_base\'s admin features behave'),
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
  );
  $form['admin_settings']['csa_base_move_sidebar'] = array(
    '#type' => 'checkbox',
    '#title' => t('Append second sidebar to first when displaying admin pages'),
    '#default_value' => $settings['csa_base_move_sidebar'],
  );
  $form['admin_settings']['breadcrumb_display_admin'] = array(
    '#type' => 'checkbox',
    '#title' => t('Always show breadcrumb on admin pages'),
    '#default_value' => $settings['breadcrumb_display_admin'],
    '#description' => t('This overwrites the general breadcrumb setting for all admin pages'),
  );

  // General Settings
  $form['general_settings'] = array(
    '#type' => 'fieldset',
    '#title' => t('General settings'),
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
    );

  // Primary Links
  $form['general_settings']['primary_links'] = array(
    '#type' => 'fieldset',
    '#title' => t('Primary links'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );
  $form['general_settings']['primary_links']['primary_links_display_style'] = array(
    '#type'          => 'radios',
    '#title'         => t('How should your Primary links be displayed?'),
    '#default_value' => $settings['primary_links_display_style'],
    '#options'       => array(
                          'menu'                      => t('As a custom menu defined in style.css'),
                          'superfish sf-horizontal'   => t('Superfish: horizontal menu with dropdowns'),
                          'superfish sf-vertical'     => t('Superfish: vertical menu (for sidebar blocks)'),
                          'tabbed-menu'               => t('Tabs: render menu as tabs'),
                        ),
  );
  $form['general_settings']['primary_links']['primary_links_allow_tree'] = array(
    '#type' => 'checkbox',
    '#title' => t('Allow nested menu tree for Primary links (required for both Superfish and Tabs)'),
    '#default_value' => $settings['primary_links_allow_tree'],
  );

  // Secondary Links
  $form['general_settings']['secondary_links'] = array(
    '#type' => 'fieldset',
    '#title' => t('Secondary links'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );
  $form['general_settings']['secondary_links']['secondary_links_display_style'] = array(
    '#type'          => 'radios',
    '#title'         => t('How should your Secondary links be displayed?'),
    '#default_value' => $settings['secondary_links_display_style'],
    '#options'       => array(
                          'menu'                      => t('As a custom menu defined in style.css'),
                          'superfish sf-horizontal'   => t('Superfish: horizontal menu with dropdowns'),
                          'superfish sf-vertical'     => t('Superfish: vertical menu (for sidebar blocks)'),
                          'tabbed-menu'               => t('Tabs: render menu as tabs'),
                        ),
  );
  $form['general_settings']['secondary_links']['secondary_links_allow_tree'] = array(
    '#type' => 'checkbox',
    '#title' => t('Allow nested menu tree for Secondary links (required for both Superfish and Tabs)'),
    '#default_value' => $settings['secondary_links_allow_tree'],
  );

  // Mission Statement
  $form['general_settings']['mission_statement'] = array(
    '#type' => 'fieldset',
    '#title' => t('Mission statement'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );
  $form['general_settings']['mission_statement']['mission_statement_pages'] = array(
    '#type'          => 'radios',
    '#title'         => t('Where should your mission statement be displayed?'),
    '#default_value' => $settings['mission_statement_pages'],
    '#options'       => array(
                          'home' => t('Display mission statement only on front page'),
                          'all' => t('Display mission statement on all pages'),
                        ),
  );

  // Breadcrumb
  $form['general_settings']['breadcrumb'] = array(
    '#type' => 'fieldset',
    '#title' => t('Breadcrumb'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );
  $form['general_settings']['breadcrumb']['breadcrumb_display'] = array(
    '#type' => 'checkbox',
    '#title' => t('Display breadcrumb'),
    '#default_value' => $settings['breadcrumb_display'],
  );
  $form['general_settings']['breadcrumb']['breadcrumb_with_title'] = array(
    '#type' => 'checkbox',
    '#title' => t('Display page title in the breadcrumb'),
    '#default_value' => $settings['breadcrumb_with_title'],
  );

  // Search Settings
  $form['general_settings']['search_container'] = array(
    '#type' => 'fieldset',
    '#title' => t('Search results'),
    '#description' => t('What additional information should be displayed on your search results page?'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );
  $form['general_settings']['search_container']['search_results']['search_snippet'] = array(
    '#type' => 'checkbox',
    '#title' => t('Display text snippet'),
    '#default_value' => $settings['search_snippet'],
  );
  $form['general_settings']['search_container']['search_results']['search_info_type'] = array(
    '#type' => 'checkbox',
    '#title' => t('Display content type'),
    '#default_value' => $settings['search_info_type'],
  );
  $form['general_settings']['search_container']['search_results']['search_info_user'] = array(
    '#type' => 'checkbox',
    '#title' => t('Display author name'),
    '#default_value' => $settings['search_info_user'],
  );
  $form['general_settings']['search_container']['search_results']['search_info_date'] = array(
    '#type' => 'checkbox',
    '#title' => t('Display posted date'),
    '#default_value' => $settings['search_info_date'],
  );
  $form['general_settings']['search_container']['search_results']['search_info_comment'] = array(
    '#type' => 'checkbox',
    '#title' => t('Display comment count'),
    '#default_value' => $settings['search_info_comment'],
  );
  $form['general_settings']['search_container']['search_results']['search_info_upload'] = array(
    '#type' => 'checkbox',
    '#title' => t('Display attachment count'),
    '#default_value' => $settings['search_info_upload'],
  );

  // Node Settings
  $form['node_type_specific'] = array(
    '#type' => 'fieldset',
    '#title' => t('Node settings'),
    '#description' => t('Here you can make adjustments to which information is shown with your content, and how it is displayed.  You can modify these settings so they apply to all content types, or check the "Use content-type specific settings" box to customize them for each content type.  For example, you may want to show the date on stories, but not pages.'),
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
  );

  // Author & Date Settings
  $form['node_type_specific']['submitted_by_container'] = array(
    '#type' => 'fieldset',
    '#title' => t('Author & date'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );
  // Default & content-type specific settings
  foreach ((array('default' => 'Default') + node_get_types('names')) as $type => $name) {
    $form['node_type_specific']['submitted_by_container']['submitted_by'][$type] = array(
      '#type' => 'fieldset',
      '#title' => t('!name', array('!name' => t($name))),
      '#collapsible' => TRUE,
      '#collapsed' => TRUE,
    );
    $form['node_type_specific']['submitted_by_container']['submitted_by'][$type]["submitted_by_author_{$type}"] = array(
      '#type'          => 'checkbox',
      '#title'         => t('Display author\'s username'),
      '#default_value' => $settings["submitted_by_author_{$type}"],
    );
    $form['node_type_specific']['submitted_by_container']['submitted_by'][$type]["submitted_by_date_{$type}"] = array(
      '#type'          => 'checkbox',
      '#title'         => t('Display date posted (you can customize this format on your <a href="@date-time-url">Date and Time</a> settings page)', array('@date-time-url' => url('admin/settings/date-time'))),
      '#default_value' => $settings["submitted_by_date_{$type}"],
    );
    // Options for default settings
    if ($type == 'default') {
      $form['node_type_specific']['submitted_by_container']['submitted_by']['default']['#title'] = t('Default');
      $form['node_type_specific']['submitted_by_container']['submitted_by']['default']['#collapsed'] = $settings['submitted_by_enable_content_type'] ? TRUE : FALSE;
      $form['node_type_specific']['submitted_by_container']['submitted_by']['submitted_by_enable_content_type'] = array(
        '#type'          => 'checkbox',
        '#title'         => t('Use custom settings for each content type instead of the default above'),
        '#default_value' => $settings['submitted_by_enable_content_type'],
      );
    }
    // Collapse content-type specific settings if default settings are being used
    else if ($settings['submitted_by_enable_content_type'] == 0) {
      $form['submitted_by'][$type]['#collapsed'] = TRUE;
    }
  }

  // Taxonomy Settings
  if (module_exists('taxonomy')) {
    $form['node_type_specific']['display_taxonomy_container'] = array(
      '#type' => 'fieldset',
      '#title' => t('Taxonomy terms'),
      '#collapsible' => TRUE,
      '#collapsed' => TRUE,
    );

    $form['node_type_specific']['display_taxonomy_container']['display'] = array(
      '#type' => 'fieldset',
      '#title' => t('Display'),
      '#collapsible' => TRUE,
      '#collapsed' => TRUE,
    );
    $form['node_type_specific']['display_taxonomy_container']['display']['taxonomy_format_delimiter'] = array(
      '#type' => 'textfield',
      '#title' => t('Delimter'),
      '#size' => 60,
      '#default_value' => $settings['taxonomy_format_delimiter'],
      '#description'   => t('Enter a custom delimiter for taxonomy terms'),
    );
    $form['node_type_specific']['display_taxonomy_container']['display']["taxonomy_format_links"] = array(
      '#type'          => 'checkbox',
      '#title'         => t('Display taxonomy terms in a links list (omitting delimiters)'),
      '#default_value' => $settings["taxonomy_format_links"],
    );
    $form['node_type_specific']['display_taxonomy_container']['display']["taxonomy_display_vocab_name"] = array(
      '#type'          => 'checkbox',
      '#title'         => t('Display vocabulary names'),
      '#default_value' => $settings["taxonomy_display_vocab_name"],
    );

    // Default & content-type specific settings
    foreach ((array('default' => 'Default') + node_get_types('names')) as $type => $name) {
      // taxonomy display per node
      $form['node_type_specific']['display_taxonomy_container']['display_taxonomy'][$type] = array(
        '#type' => 'fieldset',
        '#title'       => t('!name', array('!name' => t($name))),
        '#collapsible' => TRUE,
        '#collapsed' => TRUE,
      );
      // display
      $form['node_type_specific']['display_taxonomy_container']['display_taxonomy'][$type]["taxonomy_display_{$type}"] = array(
        '#type'          => 'select',
        '#title'         => t('When should taxonomy terms be displayed?'),
        '#default_value' => $settings["taxonomy_display_{$type}"],
        '#options'       => array(
                              '' => '',
                              'never' => t('Never display taxonomy terms'),
                              'all' => t('Always display taxonomy terms'),
                              'only' => t('Only display taxonomy terms on full node pages'),
                            ),
      );
      // format
      $form['node_type_specific']['display_taxonomy_container']['display_taxonomy'][$type]["taxonomy_format_{$type}"] = array(
        '#type'          => 'radios',
        '#title'         => t('Taxonomy display format'),
        '#default_value' => $settings["taxonomy_format_{$type}"],
        '#options'       => array(
                              'vocab' => t('Display each vocabulary on a new line'),
                              'list' => t('Display all taxonomy terms on a single line'),
                            ),
      );
      // Get taxonomy vocabularies by node type
      $vocabs = array();
      $vocabs_by_type = ($type == 'default') ? taxonomy_get_vocabularies() : taxonomy_get_vocabularies($type);
      foreach ($vocabs_by_type as $key => $value) {
        $vocabs[$value->vid] = $value->name;
      }
      // Display taxonomy checkboxes
      foreach ($vocabs as $key => $vocab_name) {
        $form['node_type_specific']['display_taxonomy_container']['display_taxonomy'][$type]["taxonomy_vocab_display_{$type}_{$key}"] = array(
          '#type'          => 'checkbox',
          '#title'         => t('Display vocabulary: '. $vocab_name),
          '#default_value' => $settings["taxonomy_vocab_display_{$type}_{$key}"],
        );
      }
      // Options for default settings
      if ($type == 'default') {
        $form['node_type_specific']['display_taxonomy_container']['display_taxonomy']['default']['#title'] = t('Default');
        $form['node_type_specific']['display_taxonomy_container']['display_taxonomy']['default']['#collapsed'] = $settings['taxonomy_enable_content_type'] ? TRUE : FALSE;
        $form['node_type_specific']['display_taxonomy_container']['display_taxonomy']['taxonomy_enable_content_type'] = array(
          '#type'          => 'checkbox',
          '#title'         => t('Use custom settings for each content type instead of the default above'),
          '#default_value' => $settings['taxonomy_enable_content_type'],
        );
      }
      // Collapse content-type specific settings if default settings are being used
      else if ($settings['taxonomy_enable_content_type'] == 0) {
        $form['display_taxonomy'][$type]['#collapsed'] = TRUE;
      }
    }
  }


  // SEO settings
  $form['seo'] = array(
    '#type' => 'fieldset',
    '#title' => t('Search engine optimization (SEO) settings'),
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
  );
  
    $form['seo']['hide_front_page_title'] = array(
    '#type' => 'checkbox',
    '#title' => t('Hide the title and author information on the frontpage?'),
    '#default_value' => $settings['hide_front_page_title'],
  );
  
  // Page titles
  $form['seo']['page_format_titles'] = array(
    '#type' => 'fieldset',
    '#title' => t('Page titles'),
    '#description'   => t('This is the title that displays in the title bar of your web browser. Your site title, slogan, and mission can all be set on your Site Information page'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );
  // front page title
  $form['seo']['page_format_titles']['front_page_format_titles'] = array(
    '#type' => 'fieldset',
    '#title' => t('Front page title'),
    '#description'   => t('Your front page in particular should have important keywords for your site in the page title'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );
  $form['seo']['page_format_titles']['front_page_format_titles']['front_page_title_display'] = array(
    '#type' => 'select',
    '#title' => t('Set text of front page title'),
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
    '#default_value' => $settings['front_page_title_display'],
    '#options' => array(
                    'title_slogan' => t('Site title | Site slogan'),
                    'slogan_title' => t('Site slogan | Site title'),
                    'title_mission' => t('Site title | Site mission'),
                    'custom' => t('Custom (below)'),
                  ),
  );
  $form['seo']['page_format_titles']['front_page_format_titles']['page_title_display_custom'] = array(
    '#type' => 'textfield',
    '#title' => t('Custom'),
    '#size' => 60,
    '#default_value' => $settings['page_title_display_custom'],
    '#description'   => t('Enter a custom page title for your front page'),
  );
  // other pages title
  $form['seo']['page_format_titles']['other_page_format_titles'] = array(
    '#type' => 'fieldset',
    '#title' => t('Other page titles'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );
  $form['seo']['page_format_titles']['other_page_format_titles']['other_page_title_display'] = array(
    '#type' => 'select',
    '#title' => t('Set text of other page titles'),
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
    '#default_value' => $settings['other_page_title_display'],
    '#options' => array(
                    'ptitle_slogan' => t('Page title | Site slogan'),
                    'ptitle_stitle' => t('Page title | Site title'),
                    'ptitle_smission' => t('Page title | Site mission'),
                    'ptitle_custom' => t('Page title | Custom (below)'),
                    'custom' => t('Custom (below)'),
                  ),
  );
  $form['seo']['page_format_titles']['other_page_format_titles']['other_page_title_display_custom'] = array(
    '#type' => 'textfield',
    '#title' => t('Custom'),
    '#size' => 60,
    '#default_value' => $settings['other_page_title_display_custom'],
    '#description'   => t('Enter a custom page title for all other pages'),
  );

  // Return theme settings form
  return $form;
}
