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
 * Job related functions.
 */

/**
 * Get the asset.
 *
 * @param $asset_id
 *  The asset ID.
 * @param $options
 */
function _csa_asset_jobs_get($asset_id, $user_id, $options = array()) {

  static $assets = array();

  $options += array(
    'limit' => 200,
    'offset' => 0,
    'fatal' => FALSE,
    'cql' => 'sortby job_id/descending',
  );

  // Setup request options.
  $request_options = array(
    'user_id' => $user_id,
    'fatal' => $options['fatal'],
  );
  unset($options['fatal']);

  // Set the options.
  $request_options['data'] = $options;

  // Do request.
  $result =  mediamosa_connector::response2array(_csa_mediamosa_request('asset/' . rawurlencode($asset_id) . '/joblist', $request_options));

  return $result['items']['item'];

}
