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
 * This include contains all code related to helper functions for assets.
 */

/**
 * Get / search for assets.
 *
 * @param array options
 *  - coll_id
 *    Array of collection IDs where assets must belong to.
 *  - hide_empty_assets
 *    TRUE to hide assets that don't have mediafiles and/or metadata.
 *  - return_asset_ids
 *    TRUE to get only asset_ids.
 *  - offset
 *    The position within the records.
 *  - limit
 *    The maximum number of items we return using this function.
 *  - order_by
 *    Column name for order.
 *  - order_direction
 *    ASC or DESC.
 */
function _csa_asset_search($options = array()) {

  $options += array(
    'hide_empty_assets' => FALSE,
    'return_asset_ids' => FALSE,
    'limit' => 200, // provide 0 to get all (be careful with this).
    'offset' => 0,
    'coll_id' => array(),
    'order_by' => 'created',
    'order_direction' => 'DESC',
    'show_collections' => FALSE,
    'hook_process' => NULL, // Provide hook to fill with result.
  );

  $max_limit = 200;

  // Set so we can do more request (one request is max 200).
  $real_limit = $options['limit'];
  $options['limit'] = $real_limit > $max_limit ? $max_limit : $real_limit;

  // Convert to text.
  $options['hide_empty_assets'] = $options['hide_empty_assets'] ? 'TRUE' : 'FALSE';
  $options['return_asset_ids'] = $options['return_asset_ids'] || !empty($options['hook_process']) ? 'TRUE' : 'FALSE';
  $options['show_collections'] = $options['show_collections'] ? 'TRUE' : 'FALSE';

  // If CQL is set, then we need to provide order by/order_direction as CQL as well.
  if (!empty($options['cql']) && !empty($options['order_by'])) {
    assert(stripos($options['cql'], 'sortby') === FALSE);

    // Add sort by.
    $options['cql'] = $options['cql'] . ' sortBy ' . $options['order_by'] . '/' . ($options['order_direction'] == 'ASC' ? 'ascending' : 'descending');
    unset($options['order_by'], $options['order_direction']);
  }

  // Copy options.
  $request_options = $options;
  unset($request_options['hook_process']);
  if (!$request_options['limit']) {
    $max_limit = 1000;
    $request_options['limit'] = $max_limit;
  }

  // Do request.
  $result =  mediamosa_connector::response2array(_csa_mediamosa_request('asset', array('data' => $request_options)));

  // Get the data.
  $assets = array('assets' => array());

  // Fill results.
  while (!empty($result['header']['item_count'])) {
    // Get the assets.
    foreach ($result['items']['item'] as $asset) {
      unset($asset['@attributes']);

      $assets['assets'][$asset['asset_id']] = $asset;
    }

    // Call hook to process result.
    if (!empty($options['hook_process']) && !empty($assets['assets'])) {
      $options['hook_process'](array_keys($assets['assets']));
      $assets = array(); // purpose of the hook is to save memory.
    }

    // If we have more, then get it.
    if ($options['limit'] && ($result['header']['item_count'] < $request_options['limit'] || count($assets['assets']) == $real_limit)) {
      break; // nothing more.
    }

    // Next set.
    $request_options['offset'] += $max_limit;

    // More to come.
    $result =  mediamosa_connector::response2array(_csa_mediamosa_request('asset', array('data' => $request_options)));
  }

  // Set total count.
  $assets['item_count_total'] = empty($result['header']['item_count_total']) ? 0 : $result['header']['item_count_total'];

  return $assets;
}

/**
 * Get the asset.
 *
 * @param $asset_id
 *  The asset ID.
 * @param $options
 */
function _csa_asset_get($asset_id, $options = array()) {

  static $assets = array();

  $options += array(
    'show_collections' => FALSE,
    'show_stills' => TRUE,
    'fatal' => FALSE,
  );

  if (!empty($assets[$asset_id][$options['show_collections']][$options['show_stills']])) {
    return $assets[$asset_id][$options['show_collections']][$options['show_stills']];
  }

  // Setup request options.
  $request_options = array(
    'data' => array(
      'show_collections' => $options['show_collections'] ? 'TRUE' : 'FALSE',
      'show_stills' => $options['show_stills'] ? 'TRUE' : 'FALSE',
    ),
    'fatal' => $options['fatal'],
  );

  // Get the asset.
  $assets[$asset_id][$options['show_collections']][$options['show_stills']] = mediamosa_connector::response2array(_csa_mediamosa_request('asset/' . $asset_id, $request_options));
  $asset = &$assets[$asset_id][$options['show_collections']][$options['show_stills']];

  if (!empty($asset['items']['item'][0]['mediafiles']) && array_key_exists('mediafile_1', $asset['items']['item'][0]['mediafiles'])) {
    for ($x = 1; !empty($asset['items']['item'][0]['mediafiles']['mediafile_' . $x]); $x++) {
      $asset['items']['item'][0]['mediafiles']['mediafile'][$x - 1]  = $asset['items']['item'][0]['mediafiles']['mediafile_' . $x];
      unset($asset['items']['item'][0]['mediafiles']['mediafile_' . $x]);
    }
  }

  return $assets[$asset_id][$options['show_collections']][$options['show_stills']];
}

/**
 * Get the owner of the asset.
 *
 * @param array $asset
 */
function _csa_asset_owner_get($asset) {

  // Go direct to the data.
  if (!empty($asset['items']['item'])) {
    $asset = reset($asset['items']['item']);
  }

  return empty($asset['owner_id']) ? FALSE : $asset['owner_id'];
}

/**
 * Returns the title for the asset or FALSE.
 */
function _csa_asset_get_title($asset, $default_title = '<no title found>', $allow_filename = TRUE) {
  // Empty?
  if (empty($asset)) {
    return $default_title;
  }

  // Go direct to the data.
  if (!empty($asset['items']['item'])) {
    $asset = reset($asset['items']['item']);
  }

  // Where to get from metadata.
  $title_metadata = array('czp' => 'titel', 'dublin_core' => 'title');

  $title = '';
  foreach ($title_metadata as $group => $name) {
    if (!empty($asset[$group][$name])) {
      $title = is_array($asset[$group][$name]) ? reset($asset[$group][$name]) : $asset[$group][$name];
    }
  }

  if ($allow_filename) {
    $filename = '';
    if (!empty($asset['mediafiles']['mediafile'])) {
      foreach ($asset['mediafiles']['mediafile'] as $mediafile) {
        // Make sure you use at least filename.
        if ($mediafile['is_original_file'] == 'TRUE') {
          $filename = $mediafile['filename'];
          break;
        }
      }
    }

    // Build title (filename) if possible.
    $title = empty($title) ? $filename : $title . ($title != $filename ? (empty($filename) ? '' : ' (' .  $filename . ')') : '');
  }

  // Return title when not empty.
  return !empty($title) ? $title : $default_title;
}

/**
 * Show asset detail page.
 *
 * @param string $asset_id
 */
function _csa_asset_detail($asset_id, $offset = -1, $mode = CSA_MODE_QUERY, $options = array()) {

  $options += array(
    'link_metadata_edit' => 'csa/asset/!asset_id/!offset/!mode/edit',
    'link_mediafile_delete' => 'csa/mediafile/!mediafile_id/delete',
  );

  // Get the asset.
  $asset = _csa_asset_get($asset_id);

  // Get the title.
  $title = _csa_asset_get_title($asset);

  // Set asset title.
  drupal_set_title($title);

  // Output.
  $output = array();

  if (empty($asset) || empty($asset['header']['item_count']) || empty($asset['items']['item'])) {
    $output['msg'] = t('Unable to find asset.');
  }
  else {
    $asset = reset($asset['items']['item']);

    // Show possible still.
    $output ['stills'] = theme('csa_asset_stills', $asset);

    // Show metadata.
    $output['metadata'] = theme('csa_asset_metadata', $asset, $offset, $mode, array('link_metadata_edit' => $options['link_metadata_edit']));

    // Get the jobs.
    $jobs = _csa_asset_jobs_get($asset['asset_id'], $asset['owner_id']);

    if (!empty($jobs)) {
      // Show jobs.
      $output['jobs'] = theme('csa_asset_jobs', $asset, $jobs, $offset, $mode);
    }

    // Show mediafiles.
    $output['mediafiles'] = theme('csa_asset_mediafiles', $asset, $offset, $mode, array('link_mediafile_delete' => $options['link_mediafile_delete'], 'link_mediafile_access' => $options['link_mediafile_access']));
  }

  return implode('', $output);
}

/**
 * Show asset edit metadata page.
 *
 * @param string $asset_id
 */
function _csa_asset_metadata_edit($asset_id, $offset = -1, $mode = CSA_MODE_QUERY, $options = array()) {

  $options += array(
    'default_destination' => '',
  );

  // Get the asset.
  $asset = _csa_asset_get($asset_id);

  // Get the title.
  $title = _csa_asset_get_title($asset);

  // Set asset title.
  drupal_set_title($title);

  // Need cached global.
  theme('csa_box_previous_next_asset', 'csa/' . arg(1) . '/asset/!asset_id/!offset/!mode/edit', $offset, $mode);

  // Output form.
  $output['metadata_form'] = drupal_get_form('_csa_asset_metadata_form', $asset_id, $offset, $mode);

  return implode('', $output);
}

/**
 * Helper function;
 * Get the assets / mediafiles using given asset_ids.
 *
 * @param array $asset_ids
 *  The assets where to gather the mediafiles from.
 *
 * @param array $options
 *
 * Warning: very memory heavy; limit your number of asset_ids to max 1000.
 *
 */
function _csa_assets_get($asset_ids, $options = array()) {
  $options += array(
    'show_collections' => TRUE,
  );

  // convert to text.
  $options['show_collections'] = ($options['show_collections'] ? 'TRUE' : 'FALSE');

  // Options for request.
  $request_options = array(
    'data' => $options,
  );

  // Do request.

  // Because its heavy on the REST server, we will do with 100 per time.

  $assets = array();
  $mediafiles = array();

  $asset_id = array_shift($asset_ids);
  while ($asset_id) {
    $request_options['data']['asset_id'] = array();
    $limit = 100;
    while ($limit-- && $asset_id) {
      $request_options['data']['asset_id'][] = $asset_id;
      $asset_id = array_shift($asset_ids);
    }

    // Do rest call.
    $result =  mediamosa_connector::response2array(_csa_mediamosa_request('assets', $request_options));

    // Build result.
    if (!empty($result['items']['item'])) {

      // Make array with key asset_ids.
      $items = array();
      foreach ($result['items']['item'] as $item) {
        $items[$item['asset_id']] = $item;
      }

      foreach ($request_options['data']['asset_id'] as $asset_id_2) {
        $asset = $items[$asset_id_2];
        unset($asset['@attributes']);

        // Copy mediafiles (old style output?)
        if (!empty($asset['mediafiles']['mediafile_1'])) {
          for ($x = 1; !empty($asset['mediafiles']['mediafile_' . $x]); $x++) {
            unset($asset['mediafiles']['mediafile_' . $x]['@attributes']);
            $asset['mediafiles']['mediafile'][$x - 1] = $asset['mediafiles']['mediafile_' . $x];
            unset($asset['mediafiles']['mediafile_' . $x]);

            if (drupal_strtoupper($asset['mediafiles']['mediafile'][$x - 1]['is_original_file']) == 'TRUE') {
              // Set the original mediafile id for this asset.
              $asset['mediafile_original'] = $asset['mediafiles']['mediafile'][$x - 1];
            }
          }
        }
        elseif (!empty($asset['mediafiles']['mediafile'])) {
          foreach ($asset['mediafiles']['mediafile'] as $key => $mediafile) {
            unset($asset['mediafiles']['mediafile'][$key]['@attributes']);

            if (drupal_strtoupper($mediafile['is_original_file']) == 'TRUE') {
              // Set the original mediafile id for this asset.
              $asset['mediafile_original'] = $mediafile;
            }
          }

          // Make sure we have original.
          assert(isset($asset['mediafile_original']));
        }

        // Copy the asset.
        $assets[$asset['asset_id']] = $asset;
      }
    }
  }

  return $assets;
}

/**
 * Delete an asset.
 *
 * @param string $asset_id
 *   The asset to delete.
 * @param string $user_id
 *   The owner of the asset.
 * @param string $mode
 *   Mode of the asset.
 *
 * @see rest call /asset/$asset_id/delete.
 */
function _csa_asset_delete($asset_id, $user_id, $options = array()) {

  // Setup options.
  $options += array(
    'fatal' => FALSE,
    'delete' => 'cascade',
  );

  // Just check.
  assert(in_array($options['delete'], array('', 'cascade')));

  // Setup request.
  $request_options = array(
    'user_id' => $user_id,
    'fatal' => $options['fatal'],
    'data' => array(
      'delete' => $options['delete'],
    ),
  );

  // Setup logging.
  $message = 'Unable to delete asset with ID @asset_id';
  $variables = array('@asset_id' => $asset_id);

  try {
    // Do rest call.
    $response = _csa_mediamosa_request_post('asset/' . rawurlencode($asset_id) . '/delete', $request_options);
  }
  catch (Exception $e) {
    _csa_log_rest_response(FALSE, $message, $variables);
    throw $e;
  }

  // Log it (if the request failed).
  return _csa_log_rest_response($response, $message, $variables);
}

/**
 * Set ACL rights on asset.
 *
 * @param string $asset_id
 * @param string $user_id
 * @param array $options
 *  - aut_app
 *  - aut_domain
 *  - aut_realm
 *  - aut_group
 *  - aut_user
 *  - replace (default TRUE)
 *
 *  @see REST call /asset/$asset_id/acl
 */
function _csa_asset_set_acl($asset_id, $user_id, $options = array()) {

  // Set default.
  $options += array(
    'replace' => TRUE,
    'fatal' => FALSE,
  );

  $request_options = array(
    'user_id' => $user_id,
    'fatal' => FALSE,
    'data' => array(
      'replace' => $options['replace'] ? 'TRUE' : 'FALSE',
    ),
  );

  $request_options = array();
  foreach (array('acl_app', 'acl_domain', 'acl_realm', 'acl_user', 'acl_group') as $subject) {
    if (!empty($options[$subject])) {
      $request_options['data'][$subject] = $options[$subject];
    }
  }

  if (empty($request_options)) {
    throw new Exception('No access rules set for _csa_asset_set_acl()');
  }

  // Setup logging.
  $message = 'Unable to set ACL on asset with ID @asset_id';
  $variables = array('@asset_id' => $asset_id);

  try {
    // Set acl rights.
    $response = _csa_mediamosa_request_post('asset/' . rawurlencode($asset_id) . '/acl', $request_options);
  }
  catch (Exception $e) {
    _csa_log_rest_response(FALSE, $message, $variables);
    throw $e;
  }

  // Log it (if the request failed).
  return _csa_log_rest_response($response, $message, $variables);
}

/**
 * Remove all ACL from asset.
 *
 * @param string $asset_id
 * @param string $user_id
 *   Owner of asset.
 * @param boolean $include_mediafiles
 *   TRUE - Remove ACL from mediafiles too
 */
function _csa_asset_delete_acl($asset_id, $user_id, $include_mediafiles = TRUE) {

  $request_options = array(
    'user_id' => $user_id,
  );

  // Set acl rights.

  // Setup logging.
  $message = 'Unable to remove ACL on asset with ID @asset_id';
  $variables = array('@asset_id' => $asset_id);

  try {
    // Do REST call.
    $response = _csa_mediamosa_request_post('asset/' . rawurlencode($asset_id) . '/acl/delete', $request_options);
  }
  catch (Exception $e) {
    _csa_log_rest_response(FALSE, $message, $variables);
    throw $e;
  }

  // Log it (if the request failed).
  $result = _csa_log_rest_response($response, $message, $variables);

  if ($result && $include_mediafiles) {
    // Get the asset.
    $asset = _csa_asset_get($asset_id, array('show_stills' => FALSE));

    if ($asset) {
      // Now loop through the mediafiles (if any) and remove those ACL.
      $asset = reset($asset['items']['item']);
      foreach ($asset['mediafiles']['mediafile'] as $mediafile) {
        _csa_asset_mediafile_delete_acl($mediafile['mediafile_id'], $mediafile['owner_id']);
      }
    }
  }

  // Done.
  return $result;
}
