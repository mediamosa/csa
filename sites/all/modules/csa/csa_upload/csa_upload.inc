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
 * @file
 * Helper functions csa_upload.
 */

/**
 * Do rest call to MediaMosa and create a upload ticket.
 *
 * @param string $coll_id
 *  Is either a string (to create the collection) or the coll_id.
 * @param string $upload_id
 */
function _csa_upload_get_upload_url($upload_id) {

  global $user;

  // Get title or collection ID.
  $coll_id = $_GET['coll_id'];
  $filename = $_GET['filename'];

  // Create upload ticket
  $action = _csa_upload_create_upload_action($coll_id, $filename, $upload_id);

  // Rebuild url.
  $parse_url = parse_url($action);

  // Split up query_string.
  $parse_url['query'] = _csa_parse_query(empty($parse_url['query']) ? '' : $parse_url['query']);

  // Set inherit acl.
  $parse_url['query']['transcode_inherits_acl'] = _csa_get_user_value_prefs($user, CSA_PREFIX_PREFS_MEDIAMANGEMENT_INHERITS_ACL) ? 'TRUE' : 'FALSE';

  // Rebuild.
  $parse_url['query'] = _csa_http_build_query($parse_url['query']);

  // We are returning json.
  _csa_json_output(array('action' => _csa_build_url($parse_url)));
  exit();
}

/**
 * Create a upload ticket.
 *
 * @param string $collection_name
 * @param string $upload_id
 *
 * @return array
 */
function _csa_upload_create_upload_action($collection_name, $filename, $upload_id) {
  global $user;
  assert($user->uid);

  try {
    // Get the user id so we can attach an owner to the asset and mediafile.
    $user_id = _csa_get_mediamosa_user_id();
    $group_id = _csa_get_mediamosa_upload_group_id();

    // Check if the collection name is also an collection.
    $collection = _csa_collection_get($collection_name);

    if (!empty($collection)) {
      $coll_id = $collection['coll_id'];
    }
    else {
      // Make sure the collection does not exists.
      $collections = _csa_get_upload_collections();

      // Now find a collection with the same name.
      $coll_id = NULL;
      foreach ($collections as $coll_id_2 => $title) {
        // If we find this collection, then we use it as our upload set.
        if (drupal_strtolower($collection_name) == drupal_strtolower($title)) {
          $coll_id = $coll_id_2;
          break;
        }
      }

      // If we didn't find it, then we create the collection.
      if (!$coll_id) {
        _csa_log_info("Creating collection '!name' for new upload set.", array('!name' => $collection_name));

        // Create collection.
        $data = array('title' => $collection_name);
        $response = _csa_mediamosa_request_post('collection/create', array('user_id' => $user_id, 'group_id' => $group_id, 'data' => $data, 'fatal' => true));

        // Get the new coll_id.
        $coll_id = (string) $response->xml->items->item->coll_id;
      }
    }

    // Create the asset.
    $response = _csa_mediamosa_request_post('asset/create', array('user_id' => $user_id, 'fatal' => true));
    $asset_id = (string) $response->xml->items->item->asset_id;

    // Now we need to put the new asset into the collection.
    $data = array('asset_id' => $asset_id);
    _csa_mediamosa_request_post(strtr('collection/#coll_id/asset_relation', array('#coll_id' => $coll_id)), array('user_id' => $user_id, 'data' => $data, 'fatal' => true));

    // Create metadata.
    $args_metadata = _csa_upload_session_metadata_get($upload_id);

    // Attach the metadata.
    _csa_mediamosa_request_post(strtr('asset/#asset_id/metadata', array('#asset_id' => $asset_id)), array('data' => $args_metadata, 'user_id' => $user_id, 'fatal' => true));

    // Now create the mediafile.
    $data_create_mediafile = array('asset_id' => $asset_id);

    // Downloadable setting.
    $data_create_mediafile['is_downloadable'] = empty($user->csa_prefs[CSA_PREFIX_PREFS_MEDIAMANGEMENT_DOWNLOADABLE]) ? 'FALSE' : 'TRUE';
    $data_create_mediafile['tag'] = empty($user->csa_prefs[CSA_PREFIX_PREFS_MEDIAMANGEMENT_TAG]) ? '' : $user->csa_prefs[CSA_PREFIX_PREFS_MEDIAMANGEMENT_TAG];

    $response = _csa_mediamosa_request_post('mediafile/create', array('data' => $data_create_mediafile, 'user_id' => $user_id, 'fatal' => true));
    $mediafile_id = (string) $response->xml->items->item->mediafile_id;

    $response = _csa_mediamosa_request_post('mediafile/'. $mediafile_id . '/uploadticket/create', array('user_id' => $user_id, 'fatal' => true));

    // Get the action.
    $action =  (string) $response->xml->items->item->action;

    // Now split up again.
    $parse_url = @parse_url($action);

    // Expect the URL to be ok, if not then not.
    if ($parse_url === FALSE) {
      return $action;
    }

    // All previous stuff went ok, add default metadata to the asset.
    $default_metadata = _csa_user_prefs_default_metadata_upload_get($user);

    // Any default metadata?
    if (!empty($default_metadata)) {
      foreach ($default_metadata as $name => $values) {
        foreach ($values as $key => $value) {
          // insert tag data.
          $default_metadata[$name][$key] = strtr($value, array('=filename' => $filename));
        }
      }

      _csa_asset_metadata_save($asset_id, $default_metadata, array('fatal' => TRUE));
    }

    // Split up the query.
    $query = _csa_parse_query(empty($parse_url['query']) ? '' : $parse_url['query']);

    // Now add options we need for upload url.
    if (!empty($user->csa_prefs[CSA_PREFIX_PREFS_MEDIAMANGEMENT_GENERATE_STILL])) {
      $query['create_still'] = 'TRUE';
      $query['still_type'] = 'NONE';
    }

    // Add possible after-upload transcodings.
    $transcodings = _csa_user_prefs_upload_transcoding_get();
    if (!empty($transcodings)) {
      $query['transcode'] = array_keys($transcodings); // Need the IDs.
    }

    // Rebuild query.
    $parse_url['query'] = _csa_http_build_query($query);

    // Return rebuild url.
    return _csa_build_url($parse_url);
  }
  catch (Exception $e) {
    _csa_log_critical('Unable to create upload ticket; expection message !message. Ask your administrator to look at your MediaMosa settings.', array('!message' => $e->getMessage()));
  }

  return FALSE;
}

/**
 * Json page that returns the current MediaMosa quota of the user.
 */
function _csa_upload_update_quota() {

  // Output the quota.
  _csa_json_output(_csa_user_get_quota());
  exit();
}

/**
 * Set the current metadata for upload batch/session.
 */
function _csa_upload_session_metadata_set($upload_id, $values) {
  // Metadata def.
  $metadatas = _csa_asset_metadata_list();

  // To get the right metadata.
  $metadata_values = array();

  // Collect the value of each metadata (if not empty).
  foreach ($metadatas as $group => $metadata) {
    foreach ($metadata as $name => $property) {
      if (isset($values[$name])) {
        $metadata_values[$name] = $values[$name];
      }
    }
  }

  // For now, session based, one set per user.
  $_SESSION['csa_metadata'] = $metadata_values;
}

/**
 * Return the current metadata for upload batch/session.
 */
function _csa_upload_session_metadata_get($upload_id) {
  // For now, session based, one set per user.
  return $_SESSION['csa_metadata'];
}

/**
 * Returns the upload ID, this is the indentifier for the upload set.
 */
function _csa_upload_id_get() {
  return 'csa_upload_id';
}

/**
 * Builds default collection name based on the day.
 */
function _csa_get_default_collection_name() {

  $count = 1;

  // If collection already exists, then lets number it.
  $text = 'Upload collection of ' . format_date(time(), 'custom', 'D, m/d/Y');

  // Get the current collections from the database.
  $collections = _csa_get_upload_collections();

  $found = TRUE;
  while ($found) {
    $found = FALSE;
    $title = $text . ($count ? ' - Set ' . $count : '');

    foreach ($collections as $collection) {
      if (drupal_strtolower($collection) == drupal_strtolower($title)) {
        $count++;
        $found = TRUE;
        break;
      }
    }
  }

  return $title;
}
