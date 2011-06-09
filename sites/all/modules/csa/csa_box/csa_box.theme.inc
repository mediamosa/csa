<?php
/**
 * CSA is open source Software.
 *
 * Copyright (C) 2011 SURFnet BV (http://www.surfnet.nl) and Kennisnet
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
 * Theme functions box module.
 */

/**
 * Theme the collection/mediafile list.
 *
 * @ingroup themeable
 */
function theme_csa_box_list($form) {

  // CSS.
  drupal_add_css(drupal_get_path('module', 'csa_box') . '/csa_box.css', 'module');

  // JS.
  drupal_add_js(drupal_get_path('module', 'csa_box') . '/csa_box.js');

  $output = $rows = array();

  // Get the options.
  $options = $form['#value']['options'];

  // Set default.
  $options += array(
    'empty' =>  t('No mediafiles found.'),
    'offset' => -1, // No offset = no extra offset parameter in link.
  );

  $page = $options['page'];
  $item_count = $options['item_count'];
  $item_limit = $options['item_limit'];

  // Get all asset_ids.
  $asset_ids = array_keys($form['#value']['assets']);

  // Now get all selected/unselected.
  $selected_assets = _csa_box_item_is_selected($options['type'], $asset_ids);

  $all_selected = TRUE;
  foreach ($selected_assets as $asset_id => $selected) {
    if ($selected == FALSE) {
      $all_selected = FALSE;
      break;
    }
  }

  // Figure out the theme for the link.
  switch ($options['type']) {
    case CSA_BOX_TYPE_SEARCH:
      $link_theme = 'csa_box_link_asset_search';
      break;

    case CSA_BOX_TYPE_UNPUBLISHED:
      $link_theme = 'csa_box_link_asset_inbox';
      break;

    default:
      assert(0);

    case CSA_BOX_TYPE_PUBLISHED:
      $link_theme = 'csa_box_link_asset_outbox';
      break;
  }

  $data = array(
    '#type' => 'checkbox',
    '#value' => $all_selected,
    '#id' => 'csa-asset-select-all',
    '#attributes' => array(
      'onclick' => 'csa_box_checkbox_toggle_all(this, "' . $options['type'] . '", ".csa-asset-checkbox");',
    ),
  );

  // Main header.
  $header = array(
    array(
      'data' => drupal_render($data),
      'class' => 'csa-asset-inbox'
    ),
    array(
      'data' => t('Filename'),
      'field' => 'filename',
    ),
    array(
      'data' => t('Title'),
    ),
    t('Collection'),
    array(
      'data' => t('Size'),
      'class' => 'csa-box-size',
    ),
    array(
      'data' => t('Upload date'),
      'class' => 'csa-box-date',
      'sort' => 'desc',
      'field' => 'date',
    ),
  );

  foreach ($form['#value']['assets'] as $asset_id => $asset) {
    // Find the first original mediafile.
    if (!isset($asset['mediafile_original'])) {
      _csa_log_critical("Asset with ID '!asset_id' does not have an original.", array('!asset_id' => $asset_id));
      continue;
    }

    // Find the collection(s).
    $collections = array();
    foreach ($asset as $name => $field) {
      if (substr($name, 0, strlen('collection_')) == 'collection_') {
        // Get the collection ID.
        $coll_id = substr($name, strlen('collection_'));
        $collections[] = $form['#value']['collections'][$coll_id]['title'];
      }
    }

    // Get the mediafile original.
    $mediafile_original = $asset['mediafile_original'];

    $row = array();
    $data = array(
      '#name' => $asset_id,
      '#type' => 'checkbox',
      '#value' => !empty($selected_assets[$asset_id]),
      '#attributes' => array(
        'class' => 'csa-asset-checkbox',
        'onclick' => "return csa_box_selectitem(this, '" . $options['type'] . "', '" . $asset_id . "');"
      ),
    );

    $title = _csa_asset_get_title($asset, '-', FALSE);

    $row[] = drupal_render($data);
    $row[] = array('data' => theme($link_theme, $asset_id, (empty($mediafile_original['filename']) ? '<no filename found>' : $mediafile_original['filename']), array('offset' => $options['offset']++)), 'class' => 'csa-box-filename');
    $row[] = check_plain($title);
    $row[] = check_plain(implode(', ', $collections));
    $row[] = array('data' => check_plain(empty($mediafile_original['metadata']['filesize']) ? '-' : format_size($mediafile_original['metadata']['filesize'])), 'class' => 'csa-box-size');
    $row[] = array('data' => check_plain(empty($asset['videotimestamp']) ? '-' : _csa_format_date($asset['videotimestamp'])));
    $rows[] = array('data' => $row);
  }

  // Table.
  $output[] = theme('csa_table', array('header' => $header, 'rows' => $rows, 'empty' => $options['empty']));

  // Add pager.
  $output[] = _csa_pager($page, $item_count, $item_limit);

  return implode('', $output);
}

/**
 * Create asset link, but for inbox.
 *
 * @param string $asset_id
 * @param string $title
 * @param array $options
 *  - offset, when > 0 offset=value is added to query array for l().
 *    @see l()
 *
 */
function theme_csa_box_link_asset_inbox($asset_id, $title, $options = array()) {
  $options['path'] = 'csa/unpublished/asset/!asset_id/!offset/!mode';
  return theme('csa_asset_link', $asset_id, $title, $options);
}

/**
 * Create asset link, but for outbox.
 *
 * @param string $asset_id
 * @param string $title
 * @param array $options
 *  - offset, when > 0 offset=value is added to query array for l().
 *    @see l()
 */
function theme_csa_box_link_asset_outbox($asset_id, $title, $options = array()) {
  $options['path'] = 'csa/published/asset/!asset_id/!offset/!mode';
  return theme('csa_asset_link', $asset_id, $title, $options);
}

/**
 * Create asset link, but for search listing.
 *
 * @param string $asset_id
 * @param string $title
 * @param array $options
 *  - offset, when > 0 offset=value is added to query array for l().
 *    @see l()
 */
function theme_csa_box_link_asset_search($asset_id, $title, $options = array()) {
  $options['path'] = 'csa/search/asset/!asset_id/!offset/!mode';
  return theme('csa_asset_link', $asset_id, $title, $options);
}

/**
 * Generates a previous and next link so we browse through results.
 */
function theme_csa_box_previous_next_asset($path, $offset, $mode) {

  static $output = NULL;

  if ($offset < 0 || !is_numeric($offset)) {
    return '';
  }

  // CSS.
  drupal_add_css(drupal_get_path('module', 'csa_box') . '/csa_box.css', 'module');

  if (isset($output)) {
    return implode('', $output);
  }

  $output = array();

  // Set limit.
  $options['limit'] = $offset == 0 ? 2 : 3;
  $options['offset'] = $offset > 0 ? $offset - 1 : 0;

  $asset_ids = array();

  // Get the asset_ids.
  switch ($mode) {
    case CSA_MODE_QUERY:
      $asset_ids = _csa_box_get_asset_listing(arg(1), $options);
      $asset_ids = empty($asset_ids['assets']) ? array() : array_keys($asset_ids['assets']);
      break;

    case CSA_MODE_SELECTION:
      $asset_ids = _csa_box_get_asset_listing_selection($options);
      break;
  }

  if (empty($asset_ids)) {
    assert(0);
    return '';
  }

  // Options for asset get.
  $options += array(
    'show_collections' => FALSE,
  );

  // Get assets.
  $asset_previous_id = $offset == 0 ? FALSE : $asset_ids[0];
  $asset_next_id = ($offset == 0 && count($asset_ids) < 2) || ($offset > 0 && count($asset_ids) < 3) ? FALSE : $asset_ids[count($asset_ids) - 1];

  // Get the assets.
  $asset_ids_2 = array();
  if ($asset_previous_id) {
    $asset_ids_2[] = $asset_previous_id;
  }
  if ($asset_next_id) {
    $asset_ids_2[] = $asset_next_id;
  }

  // Get the assets.
  $assets = empty($asset_ids_2) ? array() : _csa_assets_get($asset_ids_2, $options);
  $asset_previous = empty($assets[$asset_previous_id]) ? FALSE : $assets[$asset_previous_id];
  $asset_next = empty($assets[$asset_next_id]) ? FALSE : $assets[$asset_next_id];

  $path_previous = strtr($path, array('!asset_id' => rawurlencode($asset_previous_id), '!offset' => $offset - 1, '!mode' => rawurlencode($mode)));
  $path_next = strtr($path, array('!asset_id' => rawurlencode($asset_next_id), '!offset' => $offset + 1, '!mode' => rawurlencode($mode)));

  // Save these for usage else where (button 'save & next' f.e.).
  $GLOBALS['asset_previous_url'] = $asset_previous ? $path_previous : NULL;
  $GLOBALS['asset_next_url'] = $asset_next ? $path_next : NULL;
  $GLOBALS['asset_previous_id'] = $asset_previous_id ? $asset_previous_id : NULL;
  $GLOBALS['asset_next_id'] = $asset_next_id ? $asset_next_id : NULL;

  if (empty($asset_ids_2)) {
    return '';
  }

  $output[] = '<div class="csa-link-previous">';
  $output[] = $asset_previous ? l('<< Previous asset - ' . _csa_asset_get_title($asset_previous), $path_previous) : '&nbsp;';
  $output[] = '</div><div class="csa-link-next">';
  $output[] = $asset_next ? l('Next asset - ' . _csa_asset_get_title($asset_next) . ' >>', $path_next) : '&nbsp;';
  $output[] = '</div><br />';

  return implode('', $output);
}
