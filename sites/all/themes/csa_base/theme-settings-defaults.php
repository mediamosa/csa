<?php

function csa_base_get_default_settings($theme) {
  // Get node types
  $node_types = node_get_types('names');

  // The default values for the theme variables. Make sure $defaults exactly
  // matches the $defaults in the theme-settings.php file.
  $defaults = array(
    'csa_base_move_sidebar'                 => 1,
    'breadcrumb_display'                    => 0,
    'breadcrumb_display_admin'              => 1,
    'breadcrumb_with_title'                 => 1,
    'primary_links_display_style'           => 'tabbed-menu',
    'primary_links_allow_tree'              => 0,
    'secondary_links_display_style'         => 'menu',
    'secondary_links_allow_tree'            => 0,
    'search_snippet'                        => 1,
    'search_info_type'                      => 1,
    'search_info_user'                      => 1,
    'search_info_date'                      => 1,
    'search_info_comment'                   => 1,
    'search_info_upload'                    => 1,
    'mission_statement_pages'               => 'home',
    'hide_front_page_title'                 => 1,
    'front_page_title_display'              => 'title_slogan',
    'page_title_display_custom'             => '',
    'other_page_title_display'              => 'ptitle_slogan',
    'other_page_title_display_custom'       => '',
    'configurable_separator'                => ' | ',
    'meta_keywords'                         => '',
    'meta_description'                      => '',
    'taxonomy_display_default'              => 'only',
    'taxonomy_display_vocab_name'           => 1,
    'taxonomy_format_default'               => 'vocab',
    'taxonomy_format_links'                 => 0,
    'taxonomy_format_delimiter'             => ', ',
    'taxonomy_enable_content_type'          => 0,
    'submitted_by_author_default'           => 0,
    'submitted_by_date_default'             => 0,
    'submitted_by_enable_content_type'      => 0,
  );

  // Make the default content-type settings the same as the default theme settings,
  // so we can tell if content-type-specific settings have been altered.
  $defaults = array_merge($defaults, theme_get_settings());

  // Set the default values for content-type-specific settings
  foreach ($node_types as $type => $name) {
    $defaults["taxonomy_display_{$type}"]         = $defaults['taxonomy_display_default'];
    $defaults["taxonomy_format_{$type}"]          = $defaults['taxonomy_format_default'];
    $defaults["submitted_by_author_{$type}"]      = $defaults['submitted_by_author_default'];
    $defaults["submitted_by_date_{$type}"]        = $defaults['submitted_by_date_default'];
  }

  // Get default theme settings.
  $settings = theme_get_settings($theme);

  // Don't save the toggle_node_info_ variables
  if (module_exists('node')) {
    foreach (node_get_types() as $type => $name) {
      unset($settings['toggle_node_info_'. $type]);
    }
  }
  
  // Save default theme settings
  variable_set(
    str_replace('/', '_', 'theme_'. $theme .'_settings'),
    array_merge($defaults, $settings)
  );
  
  // Force refresh of Drupal internals
  theme_get_setting('', TRUE);
  
  return $defaults;
}