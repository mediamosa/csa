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
 * Theme functions for collections.
 */

/**
 * Shows basic info of collection.
 *
 * @param string $asset
 * @param array $options
 */
function theme_csa_collection($collection, $offset = -1, $options = array()) {

  // CSS.
  drupal_add_css(drupal_get_path('module', 'csa_collection') . '/csa_collection.css', 'module');

  // Default settings.
  $options += array(
    'link_edit' => 'csa/collection/!coll_id/!offset/edit',
  );

  // Start empty.
  $rows = array();

  $fields = array('title', 'description'/*, 'owner_id', 'group_id'*/);
  foreach ($fields as $field) {
    $rows[] = array(
      array('data' => check_plain(t(_csa_name2title($field))), 'class' => 'csa-collection-metadata-name'),
      array('data' => check_plain($collection[$field]), 'class' => 'csa-collection-metadata-value')
    );
  }

  // Links after caption
  $caption_links[] = l('Edit', strtr($options['link_edit'], array('!coll_id' => rawurlencode($collection['coll_id']), '!offset' => (int) $offset)));

  return theme('csa_table', array('rows' => $rows, 'caption' => t('Properties'), 'caption_links' => $caption_links, 'empty' => t('This collection has no data.')));
}

/**
 * Theme the collection/mediafile list.
 *
 * @ingroup themeable
 */
function theme_csa_collection_list($collections, $options) {

  // CSS.
  drupal_add_css(drupal_get_path('module', 'csa_collection') . '/csa_collection.css', 'module');

  // Set default.
  $options += array(
    'empty' =>  t('No collections were found.'),
  );

  // Pager stuff.
  $page = $options['page'];
  $item_count = $options['item_count'];
  $item_limit = $options['item_limit'];
  $offset = $options['offset'];

  $rows = array();

  foreach ($collections as $collection) {
    $row = array();

    $row[] = theme('csa_collection_link', $collection['coll_id'], _csa_collection_get_title($collection), array('offset' => $offset++, 'show_filter_link' => FALSE));
    $row[] = array('data' => check_plain(empty($collection['numofvideos']) ? '0' : $collection['numofvideos']), 'class' => 'csa-collection-numofvideos');
    $row[] = check_plain(empty($collection['created']) ? '-' : _csa_format_date($collection['created']));

    $rows[] = array('data' => $row);
  }

  // Main header.
  $header = array(t('Collection'), array('data' => t('# of assets'), 'class' => 'csa-collection-numofvideos'), array('data' => t('Creation Date'), 'class' => 'csa-collection-date'));

  // Table.
  $output[] = theme('csa_table', array('header' => $header, 'rows' => $rows, 'empty' => $options['empty']));

  // Add pager.
  $output[] = _csa_pager($page, $item_count, $item_limit);

  return implode('', $output);
}

/**
 * Shows the asset listing in the collection.
 *
 * @param string $asset
 * @param array $options
 */
function theme_csa_collection_assets($collection, $assets, $options = array()) {

  // CSS.
  drupal_add_css(drupal_get_path('module', 'csa_collection') . '/csa_collection.css', 'module');

  // Set default.
  $options += array(
    'empty' =>  t('This collection has no assets.'),
    'offset' => -1, // No offset = no extra offset parameter in link.
    'page' => 0,
    'item_count' => 0,
    'item_limit' => CSA_COLLECTION_ASSETS_PER_PAGE,
  );

  $page = $options['page'];
  $item_count = $options['item_count'];
  $item_limit = $options['item_limit'];

  // Main header.
  $header = array(t('Filename'), array('data' => t('Size'), 'class' => 'csa-collection-size'), array('data' => t('Upload date'), 'class' => 'csa-collection-date'));

  // Add assets.
  $rows = array();
  foreach ($assets as $asset) {
    $row = array();
    $row[] = array('data' => theme('csa_asset_link', $asset['asset_id'], (empty($asset['mediafile_original']['filename']) ? '<no filename found>' : $asset['mediafile_original']['filename'])), 'class' => 'csa-collection-filename');
    $row[] = array('data' => check_plain(empty($asset['mediafile_original']['metadata']['filesize']) ? '-' : format_size($asset['mediafile_original']['metadata']['filesize'])), 'class' => 'csa-collection-size');
    $row[] = check_plain(empty($asset['videotimestamp']) ? '-' : _csa_format_date($asset['videotimestamp']));
    $rows[] = array('data' => $row);
  }

  // Table.
  $output[] = theme('csa_table', array('caption' => t('Assets (' . $item_count . ')'), 'header' => $header, 'rows' => $rows, 'empty' => $options['empty']));

  // Add pager.
  $output[] = _csa_pager($page, $item_count, $item_limit);

  return implode('', $output);
}

/**
 * Generates previous/next link.
 *
 * @param string $path
 *   csa/collection/!coll_id/!offset
 *   csa/collection/!coll_id/!offset/edit
 * @param string $coll_id
 * @param integer $offset
 */
function theme_csa_collection_previous_next_collection($path, $offset) {
  static $output = NULL;

  if ($offset < 0 || !is_numeric($offset)) {
    return '';
  }

  // CSS.
  drupal_add_css(drupal_get_path('module', 'csa_collection') . '/csa_collection.css', 'module');

  if (isset($output)) {
    return implode('', $output);
  }

  $output = array();

  // Set limit.
  $options['limit'] = $offset == 0 ? 2 : 3;
  $options['offset'] = $offset > 0 ? $offset - 1 : 0;

  // Get the collections.
  $collections = _csa_collection_get_collection_listing($options);

  if (empty($collections)) {
    assert(0);
    return '';
  }

  // Get the collection IDs.
  $coll_ids = array_keys($collections['collections']);

  // Get collection ids.
  $collection_previous_id = $offset == 0 ? FALSE : $coll_ids[0];
  $collection_next_id = ($offset == 0 && count($coll_ids) < 2) || ($offset > 0 && count($coll_ids) < 3) ? FALSE : $coll_ids[count($coll_ids) - 1];

  // Get the collections.
  $collection_previous = empty($collections['collections'][$collection_previous_id]) ? FALSE : $collections['collections'][$collection_previous_id];
  $collection_next = empty($collections['collections'][$collection_next_id]) ? FALSE : $collections['collections'][$collection_next_id];

  $path_previous = strtr($path, array('!coll_id' => rawurlencode($collection_previous_id), '!offset' => $offset - 1));
  $path_next = strtr($path, array('!coll_id' => rawurlencode($collection_next_id), '!offset' => $offset + 1));

  // Save these for usage else where (button 'save & next' f.e.).
  $GLOBALS['collection_previous_url'] = $collection_previous ? url($path_previous) : NULL;
  $GLOBALS['collection_next_url'] = $collection_next ? url($path_next) : NULL;
  $GLOBALS['collection_previous_id'] = $collection_previous_id ? $collection_previous_id : NULL;
  $GLOBALS['collection_next_id'] = $collection_next_id ? $collection_next_id : NULL;

  if (empty($coll_ids)) {
    return '';
  }

  $output[] = '<div class="csa-link-previous">';
  $output[] = $collection_previous ? l('<< Previous collection - ' . _csa_collection_get_title($collection_previous), $path_previous) : '&nbsp;';
  $output[] = '</div><div class="csa-link-next">';
  $output[] = $collection_next ? l('Next collection - ' . _csa_collection_get_title($collection_next) . ' >>', $path_next) : '&nbsp;';
  $output[] = '</div>';

  return implode('', $output);
}
