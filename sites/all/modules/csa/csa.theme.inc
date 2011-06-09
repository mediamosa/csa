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
 * Contains all global theme functions.
 */

/**
 * Own table theme class.
 *
 * @param array $options
 */
function theme_csa_table($options = array()) {

  // Default.
  $options += array(
    'header' => array(),
    'rows' => array(),
    'attributes' => array(),
    'caption' => NULL,
    'caption_links' => NULL,
    'empty' => '', // When no rows are found.
  );

  $header = $options['header'];
  $rows = $options['rows'];
  $attributes = $options['attributes'];
  $caption = $options['caption'];

  // Caption.
  if (!empty($options['caption'])) {
    $output['caption'] = theme('csa_caption_links', $options);
  }

  if (!empty($options['empty']) && empty($rows)) {
    $rows[] = array(array('data' => $options['empty'], 'colspan' => empty($header) ? 1 : count($header)));
  }

  $output['table'] = theme('table', $header, $rows, $attributes);
  return implode('', $output);
}

/**
 * Show caption + links.
 *
 * @param array $options
 */
function theme_csa_caption_links($options) {

  $output = array();

  if (!empty($options['caption'])) {
    $output[] = check_plain($options['caption']);
  }

  if (!empty($options['caption_links'])) {
    $output[] = '<span class="csa-caption-links">(' . implode(', ', $options['caption_links']) . ')</span>';
  }

  return empty($options['caption']) ? implode(' ', $output) : '<h3>' . implode(' ', $output) . '</h3>';
}

/**
 * Display a link to a asset.
 *
 * @param string $asset_id
 * @param string $title
 * @param array $options
 *  @See l() for options.
 */
function theme_csa_asset_link($asset_id, $title, $options = array()) {

  // If module is not enabled, then we can't show the asset.
  if (!module_exists('csa_asset') && (empty($options['path']) || $options['path'] == 'csa/asset/!asset_id/!offset/!mode')) {
    return $title;
  }

  // Merge in defaults.
  $options += array(
    'attributes' => array(),
    'html' => FALSE,
    'path' => 'csa/asset/!asset_id/!offset/!mode',
    'query' => array(),
    'offset' => -1,
    'mode' => CSA_MODE_QUERY,
  );

  $offset = $options['offset'];
  unset($options['offset']);
  $mode = $options['mode'];
  unset($options['mode']);

  $path = strtr($options['path'], array('!asset_id' => rawurlencode($asset_id), '!offset' => rawurlencode($offset), '!mode' => rawurlencode($mode)));
  unset($options['path']);

  return l($title, $path, $options);
}

/**
 * Display a link to a collection.
 *
 * @param string $asset_id
 * @param string $title
 * @param array $options
 *  @See l() for options.
 */
function theme_csa_collection_link($coll_id, $title, $options = array()) {

  // If module is not enabled, then we can't show the collection.
  if (!module_exists('csa_collection') && (empty($options['path']) || $options['path'] == 'csa/collection/!coll_id/!offset')) {
    return $title;
  }

  // Merge in defaults.
  $options += array(
    'attributes' => array(),
    'html' => FALSE,
    'path' => 'csa/collection/!coll_id/!offset',
    'query' => array(),
    'offset' => -1,
    'show_filter_link' => FALSE,
  );

  $offset = $options['offset'];
  unset($options['offset']);

  $path = strtr($options['path'], array('!coll_id' => rawurlencode($coll_id), '!offset' => rawurlencode($offset)));
  unset($options['path']);

  $collection_link_options = array();
  if ($options['show_filter_link']) {
    $collection_link_options[] = l(t('search'), strtr('csa/collection/!coll_id/search', array('!coll_id' => rawurlencode($coll_id))));
  }

  return l($title, $path, $options) . (empty($collection_link_options) ? '' : ' (' . implode(', ', $collection_link_options) . ')');
}

/**
 * Show the main still (for now)
 *
 * @param array $asset
 * @param array $options
 */
function theme_csa_asset_stills($asset, $options = array()) {
  if (empty($asset['vpx_still_url'])) {
    return;
  }

  // Get the url.
  $url = $asset['vpx_still_url'];

  $build[] = '<div>';

  // Return one themed still.
  $build[] = theme('csa_asset_still', $url);

  $build[] = '</div>';

  return implode('', $build);
}

/**
 * Themed still
 *
 * @param $url
 * @param $options
 */
function theme_csa_asset_still($url, $options = array()) {
  return theme('image', $url, '', '', NULL, FALSE);
}

/**
 * Show metadata of asset.
 *
 * @param string $asset
 * @param array $options
 */
function theme_csa_asset_metadata($asset, $offset = -1, $mode = CSA_MODE_QUERY, $options = array()) {
  $options += array(
    'link_metadata_edit' => 'csa/asset/!asset_id/!offset/!mode/edit/',
  );

  // Get all metadata options.
  $asset_metadata_list = _csa_asset_metadata_list_misc();

  $rows = array();
  $sections = array();

  foreach ($asset_metadata_list['metadata_groups'] as $group => $info) {

    // Go through data, only add stuff that is not empty.
    foreach ($asset_metadata_list['metadata'][$group] as $name => $metadata_info) {
      if (empty($asset[$group][$name])) {
        continue;
      }

      if (empty($sections[$info['title']])) {
        $sections[$info['title']] = array();
      }

      if (!is_array($asset[$group][$name])) {
        if (!empty($asset[$group][$name])) {
          $sections[$info['title']][] = array(
            array('data' => check_plain(t(_csa_name2title($name))), 'class' => 'csa-asset-metadata-name'),
            array('data' => check_plain($asset[$group][$name]), 'class' => 'csa-asset-metadata-value')
          );
        }
      }
      else {
        foreach ($asset[$group][$name] as $value) {
          if (!empty($value)) {
            $sections[$info['title']][] = array(
              array('data' => check_plain(t(_csa_name2title($name))), 'class' => 'csa-asset-metadata-name'),
              array('data' => check_plain($value), 'class' => 'csa-asset-metadata-value')
            );
          }
        }
      }
    }
  }

  // Links after caption.
  $caption_links[] = l('Edit metadata', strtr($options['link_metadata_edit'], array('!asset_id' => rawurlencode($asset['asset_id']), '!offset' => (int) $offset, '!mode' => rawurlencode($mode))));

  $output[] = theme('csa_caption_links', array('caption_links' => $caption_links));

  foreach ($sections as $title => $rows) {
    $output[] = theme('fieldset', array('#collapsible' => TRUE, '#collapsed' => FALSE, '#title' => $title, '#value' => theme('csa_table', array('rows' => $rows))));
  }

  return theme('fieldset', array('#collapsible' => TRUE, '#collapsed' => FALSE, '#title' => t('Metadata'), '#value' => implode('', $output)));
}

/**
 * Show the jobs.
 *
 * @param $asset
 * @param $offset
 * @param $mode
 * @param $options
 */
function theme_csa_asset_jobs($asset, $jobs, $offset = -1, $mode = CSA_MODE_QUERY, $options = array()) {
  // Our output.
  $output = array();

  // Set title.
  $title = t('Jobs') . ' (' . count($jobs) . ')';

  $caption_links = array();
  $caption = '';

  $rows = array();
  foreach ($jobs as $job) {
    $rows[] = array(
      $job['id'],
      $job['job_type'],
      $job['status'],
      $job['started_unix'] ? format_date($job['started_unix'], 'small') : '',
      (string) $job['progress'] * 100 . '%',
      (string) $job['owner'],
      (string) $job['error_description'],
    );
  }
  $header = array(
    t('ID'),
    t('Type'),
    t('Status'),
    t('Started'),
    t('Progress'),
    t('Owner'),
    t('Description'),
  );


  $output[] = theme('csa_table', array('header' => $header, 'rows' => $rows, 'caption' => $caption, 'caption_links' => $caption_links, 'empty' => t('No jobs found.')));

  return theme('fieldset', array('#collapsible' => TRUE, '#collapsed' => TRUE, '#title' => $title, '#value' => implode('', $output)));
}

/**
 * Show the mediafiles metadata of the asset.
 *
 * @param array $asset
 * @param array $options
 */
function theme_csa_asset_mediafiles($asset,  $offset = -1, $mode = CSA_MODE_QUERY, $options = array()) {

  $options += array(
    'link_mediafile_delete' => 'csa/mediafile/!mediafile_id/delete',
    'link_mediafile_access' => 'csa/mediafile/!mediafile_id/access',
  );

  // Our output.
  $output = array();

  // Set title.
  $title =  t('Mediafiles') . ' (' . (empty($asset['mediafiles']['mediafile']) ? 0 : count($asset['mediafiles']['mediafile'])) . ')';

  $options_mediafile = array(
    'link_mediafile_delete' => $options['link_mediafile_delete'],
    'link_mediafile_access' => $options['link_mediafile_access'],
  );

  if (empty($asset['mediafiles']['mediafile'])) {
    $output[] = t('This asset has no mediafiles.');
  }
  else {
    // Show original(s) at the top.
    $originals = array(); // should be one.
    foreach ($asset['mediafiles']['mediafile'] as $mediafile) {
      if ($mediafile['is_original_file'] != 'TRUE') {
        continue;
      }

      $originals[] = theme('csa_asset_mediafile', $asset, $mediafile, $offset, $mode, $options_mediafile);
    }

    if (!empty($originals)) {
      $output[] = theme('fieldset', array('#collapsible' => TRUE, '#collapsed' => FALSE, '#title' => t('Original mediafile'), '#value' => implode('<br />', $originals)));
    }
    else {
      $output[] = t('Warning; no original mediafile found');
    }

    // Show non-original(s) at the bottom.
    $transcodes = array();
    foreach ($asset['mediafiles']['mediafile'] as $mediafile) {
      if ($mediafile['is_original_file'] == 'TRUE') {
        continue;
      }

      $transcodes[] = theme('csa_asset_mediafile', $asset, $mediafile, $offset, $mode, $options_mediafile);
    }

    if (!empty($transcodes)) {
      $output[] = theme('fieldset', array('#collapsible' => TRUE, '#collapsed' => TRUE, '#title' => t('Transcodes') . ' (' . count($transcodes) . ')', '#value' => implode('<br />', $transcodes)));
    }
  }

  return theme('fieldset', array('#collapsible' => TRUE, '#collapsed' => FALSE, '#title' => $title, '#value' => implode('', $output)));
}

/**
 * Show properties mediafile.
 *
 * @param array $mediafile
 * @param array $options
 */
function theme_csa_asset_mediafile($asset, $mediafile, $offset = -1, $mode = CSA_MODE_QUERY, $options = array()) {

  $options += array(
    'link_mediafile_delete' => 'csa/mediafile/!mediafile_id/delete',
    'link_mediafile_access' => 'csa/mediafile/!mediafile_id/access',
  );

  // Links after caption.
  $caption_links = array();
  $caption = $mediafile['filename'];
  $rows = array();

  foreach ($mediafile['metadata'] as $name => $value) {
    // Special cases.
    if ($name == 'filesize') {
      $value = format_size($value);
    }

    if (!empty($value)) {
      $rows[] = array(
        array('data' => check_plain(t(_csa_name2title($name))), 'class' => 'csa-asset-mf-metadata-name'),
        array('data' => check_plain($value), 'class' => 'csa-asset-mf-metadata-value')
      );
    }
  }

  // Links after caption.
  $caption_links[] = l('Delete mediafile', strtr($options['link_mediafile_delete'], array('!mediafile_id' => rawurlencode($mediafile['mediafile_id']))), array('query' => drupal_get_destination()));
  $caption_links[] = l('Edit access', strtr($options['link_mediafile_access'], array('!mediafile_id' => rawurlencode($mediafile['mediafile_id']))), array('query' => drupal_get_destination()));

  $output[] = theme('csa_table', array('rows' => $rows, 'caption' => $caption, 'caption_links' => $caption_links, 'empty' => t('This mediafile has no metadata.')));

  // Get access.
  $access = _csa_asset_mediafile_get_acl($mediafile['mediafile_id'], $mediafile['owner_id']);

  if ($access) {
    $output[] = theme('csa_asset_mediafile_access', $asset, $mediafile, $access, $offset, $mode);
  }

  return implode('', $output);
}

/**
 * Show the jobs.
 *
 * @param $asset
 * @param $offset
 * @param $mode
 * @param $options
 */
function theme_csa_asset_mediafile_access($asset, $mediafile, $access, $offset = -1, $mode = CSA_MODE_QUERY, $options = array()) {

  $rights = array();
  foreach ($access as $right) {
    if (!empty($right['aut_domain'])) {
      $rights['domains'][] = $right['aut_domain'];
    }
    if (!empty($right['aut_realm'])) {
      $rights['realms'][] = $right['aut_realm'];
    }
    if (!empty($right['aut_app'])) {
      $rights['apps'][] = $right['aut_app'];
    }
  }

  $rows = array();
  if (!empty($rights['realms'])) {
    $rows[] = array(
      array('data' => t('Realms'), 'class' => 'csa-asset-mf-metadata-name'),
      array('data' => check_plain(implode(', ', $rights['realms'])), 'class' => 'csa-asset-mf-metadata-value')
    );
  }
  if (!empty($rights['domains'])) {
    $rows[] = array(
      array('data' => t('Domains'), 'class' => 'csa-asset-mf-metadata-name'),
      array('data' => check_plain(implode(', ', $rights['domains'])), 'class' => 'csa-asset-mf-metadata-value')
    );
  }
  if (!empty($rights['apps'])) {
    $rows[] = array(
      array('data' => t('Apps'), 'class' => 'csa-asset-mf-metadata-name'),
      array('data' => check_plain(implode(', ', $rights['apps'])), 'class' => 'csa-asset-mf-metadata-value')
    );
  }

  $caption_links[] = l('Edit access', strtr($options['link_mediafile_access'], array('!mediafile_id' => rawurlencode($mediafile['mediafile_id']))), array('query' => drupal_get_destination()));

  return theme('fieldset', array('#collapsible' => TRUE, '#collapsed' => TRUE, '#title' => t('Access rules'), '#value' => theme('csa_table', array('rows' => $rows))));
}
