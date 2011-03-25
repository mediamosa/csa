<?php
// $Id$

/**
 * MediaMosa is a Full Featured, Webservice Oriented Media Management and
 * Distribution platform (http://www.vpcore.nl)
 *
 * Copyright (C) 2009 SURFnet BV (http://www.surfnet.nl) and Kennisnet
 * (http://www.kennisnet.nl)
 *
 * MediaMosa is based on the open source Drupal platform and
 * was originally developed by Madcap BV (http://www.madcap.nl)
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
 * Get a list of all applications.
 */
function _csa_app_get_list($options = array()) {

  $apps = array();

  // Do request.
  $result =  mediamosa_connector::response2array(_csa_mediamosa_request('acl/app'));

  // Get the result.
  if (!empty($result['items']['item'])) {
    foreach ($result['items']['item'] as $item) {
      $apps[$item['app_id']] = $item['name'];
    }
  }

  return $apps;
}

/**
 * Get a list of all my transcodes.
 */
function _csa_app_transcode_list($options = array()) {

  $transcodings = array();

  // Do request.
  $result =  mediamosa_connector::response2array(_csa_mediamosa_request('transcode/profile'));

  // Get the result.
  if (!empty($result['items']['item'])) {
    foreach ($result['items']['item'] as $item) {
      $transcodings[$item['profile_id']] = $item['profile'];
    }
  }

  return $transcodings;
}

