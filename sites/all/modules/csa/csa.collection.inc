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
 * This include contains all code related to helper functions for collections.
 */

// Submit text buttons.
define('CSA_SUBMIT_TEXT_COLLECTION_SELECT', 'Select collection');
define('CSA_SUBMIT_TEXT_COLLECTION_SELECT_CANCEL', 'Cancel');


/**
 * Get collection.
 *
 * @param $coll_id
 */
function _csa_collection_get($coll_id, $options = array()) {

  static $collections = array();

  $options += array(
    'fatal' => FALSE,
    'skip_header' => TRUE, // return the first item in the item array.
    'get_acl' => FALSE, // return with ACL.
  );

  if (empty($coll_id)) {
    return FALSE;
  }

  // Get skip header setting.
  $skip_header = $options['skip_header'];
  unset($options['skip_header']);

  // Cache.
  if (empty($collections[$coll_id])) {
    $collections[$coll_id] = mediamosa_connector::response2array(_csa_mediamosa_request('collection/' . rawurlencode($coll_id), $options));
  }

  if (!empty($options['get_acl'])) {

  }

  return (empty($collections[$coll_id]['items']['item']) ? FALSE : ($skip_header ? reset($collections[$coll_id]['items']['item']) : $collections[$coll_id]));
}

/**
 * Return the ACL rules of collection.
 *
 * @param string $coll_id
 * @param array $options
 */
function _csa_collection_get_acl($coll_id, $user_id, $options = array()) {
  $options += array(
    'fatal' => FALSE,
  );

  if (empty($coll_id)) {
    return array();
  }

  $request_options = array(
    'user_id' => $user_id,
    'fatal' => $options['fatal'],
    'data' => array(
    ),
  );

  $acl = mediamosa_connector::response2array(_csa_mediamosa_request('collection/' . rawurlencode($coll_id) . '/acl', $request_options), $options);

  // Get the rules.
  $acl_rules = array();

  if (!empty($acl['items']['item'])) {
    foreach ($acl['items']['item'] as $item)  {
      if (!empty($item['acl_app'])) {
        $acl_rules['acl_app'][] = $item['acl_app'];
      }
    }
  }

  return $acl_rules;
}

/**
 * Returns TRUE / FALSE for publication status.
 *
 * @param string $coll_id
 */
function _csa_collection_is_published($coll_id, $user_id) {

  // Get ACL.
  $collection_acl = _csa_collection_get_acl($coll_id, $user_id);

  // Any aut_apps?
  return !empty($collection_acl['acl_app']);
}

/**
 * Get collection by title.
 *
 * Note: title is not unique, will return multiple when more than one collection
 *       has the same title for the current app_id.
 *
 * @param string $title
 *   The title to search on.
 * @param array $options
 *   - order_by
 *     Order on field.
 *   - order_direction
 *     Either 'asc' or 'desc'.
 *
 * @return Array
 *   Collection(s) or FALSE
 *
 */
function _csa_collection_get_by_title($title, $options = array()) {

  $options += array(
    'order_by' => 'created',
    'order_direction' => 'asc',
  );

  $data = array(
    'cql' => 'title == "^' . addslashes($title) .'^" sortBy ' . $options['order_by'] . '/' . ($options['order_direction'] == 'asc' ? 'ascending' : 'descending'),
  );

  // Do rest call.
  $response = mediamosa_connector::response2array(_csa_mediamosa_request('collection', array('data' => $data)));

  return (empty($response['items']['item']) ? FALSE : $response['items']['item']);
}

/**
 * Create a collection.
 *
 * @param string $user_id
 *   The owner for the new collection.
 * @param array $options
 *   - title
 *   - description
 *   - group_id
 *   - category (boolean)
 *   - isprivate (boolean)
 *   - private (boolean)
 *   - public (boolean)
 *   - public_assign (boolean)
 *
 *   @See collection create REST call.
 */
function _csa_collection_create($user_id, $options) {

  $data = array(
    'user_id' => $user_id,
  );

  foreach ($options as $name => $value) {
    switch ($name) {
      case 'category':
      case 'isprivate':
      case 'private':
      case 'public':
      case 'public_assign':
        $data[$name] = $value ? 'TRUE' : 'FALSE';
        break;

      default:
        $data[$name] = $value;
        break;
    }
  }

  // Setup logging.
  $message = "Unable to create collection";
  $variables = array();

  try {
    // Set acl rights.
    $response = _csa_mediamosa_request_post('collection/create', array('data' => $data));
  }
  catch (Exception $e) {
    _csa_log_rest_response(FALSE, $message, $variables);
    throw $e;
  }

  // Log it (if the request failed).
  if (_csa_log_rest_response($response, $message, $variables)) {
     // Return FALSE or coll_id.
    return (empty($response->xml->items->item->coll_id) ? FALSE : $response->xml->items->item->coll_id);
  }

  return FALSE;
}

/**
 * Get a list of collections.
 *
 * @param array options
 *  - group_ids (string)
 *    Provide array of group id to select collections where this group_id
 *    belongs to.
 *  - calculate_total_count
 *    TRUE / FALSE to calculate the total amount of collections.
 *  - offset
 *    The position within the records.
 *  - limit
 *    The maximum number of items we return using this function.
 *  - order_by
 *    Column name for order.
 *  - order_direction
 *    ASC or DESC.
 *  - for_select
 *    TRUE / FALSE return coll_id => name listing.
 */
function _csa_collection_search($options = array()) {

  $options += array(
    'limit' => 200,
    'offset' => 0,
    'calculate_total_count' => TRUE,
    'order_by' => 'title',
    'order_direction' => 'ASC',
    'group_ids' => array(),
    'for_select' => FALSE,
  );

  // Set so we can do more request (one request is max 200).
  $real_limit = $options['limit'];
  $options['limit'] = 200;

  // Convert to text.
  $options['calculate_total_count'] = $options['calculate_total_count'] ? 'TRUE' : 'FALSE';

  // Or'ed;
  $cql = array();

  // Groups ids.
  assert(is_array($options['group_ids']));

  foreach ($options['group_ids'] as $group_id) {
    $cql[] = 'group_id == "^' . $group_id . '^"';
  }

  // Add CQL
  if (!empty($cql)) {
    $data['cql'] = implode(' OR ', $cql);
  }

  // Do request.
  $result =  mediamosa_connector::response2array(_csa_mediamosa_request('collection', array('data' => $options)));

  if ($options['for_select']) {
    $collections = array();

    // Fill results.
    while (!empty($result['header']['item_count'])) {
      foreach ($result['items']['item'] as $collection) {
        $collections[$collection['coll_id']] = _csa_collection_get_title($collection);
      }

      // If we have more, then get it.
      if ($result['header']['item_count'] < $options['limit'] || count($collections) == $real_limit) {
        break; // nothing more.
      }

      // Next set.
      $options['offset'] += 200;

      // More to come.
      $result =  mediamosa_connector::response2array(_csa_mediamosa_request('collection', array('data' => $options)));
    }

    return $collections;
  }

  // Get the data.
  $collections = array('collections' => array());

  // Fill results.
  while (!empty($result['header']['item_count'])) {
    // Get the assets.
    foreach ($result['items']['item'] as $collection) {
      unset($collection['@attributes']);

      $collections['collections'][$collection['coll_id']] = $collection;

      if (count($collections['collections']) == $real_limit) {
        break; // done.
      }
    }

    // If we have more, then get it.
    if ($result['header']['item_count'] < $options['limit'] || count($collections['collections']) == $real_limit) {
      break; // nothing more.
    }

    // Next set.
    $options['offset'] += 200;

    // More to come.
    $result =  mediamosa_connector::response2array(_csa_mediamosa_request('collection', array('data' => $options)));
  }

  // Set total count.
  $collections['item_count_total'] = empty($result['header']['item_count_total']) ? 0 : $result['header']['item_count_total'];

  // Done.
  return $collections;
}

/**
 * Returns the title of the collection.
 *
 * @param array $collection
 */
function _csa_collection_get_title($collection, $default = '<no title found>') {
  return empty($collection['title']) ? $default : $collection['title'];
}

/**
 * Add assets to collection.
 *
 * @param array $asset_ids
 * @param string $coll_id
 * @param string $user_id
 * @param array $options
 */
function _csa_collection_add_assets($asset_ids, $coll_id, $user_id, $options) {

  $options += array(
    'fatal' => FALSE,
  );

  // Get collection.
  $collection = _csa_collection_get($coll_id, array('fatal' => $options['fatal']));

  $request_options = array(
    'user_id' => $user_id,
    'fatal' => $options['fatal'],
    'data' => array(
      'asset_id' => $asset_ids,
    ),
  );

  // Setup logging.
  $message = "Unable to add assets to collection '@name'";
  $variables = array('@name' => _csa_collection_get_title($collection));

  try {
    // Set acl rights.
    $response = _csa_mediamosa_request_post('collection/' . rawurlencode($coll_id) . '/asset_relation', $request_options);
  }
  catch (Exception $e) {
    _csa_log_rest_response(FALSE, $message, $variables);
    throw $e;
  }

  // Log it (if the request failed).
  $result = _csa_log_rest_response($response, $message, $variables);
  // Return the number of assets really removed.
  if ($result) {
    return count($response->xml->items->item);
  }

  return FALSE;

}

/**
 * Remove assets from collection.
 *
 * @param array $asset_ids
 * @param string $coll_id
 * @param string $user_id
 * @param array $options
 */
function _csa_collection_remove_assets($asset_ids, $coll_id, $user_id, $options) {

  $options += array(
    'fatal' => FALSE,
  );

  // Get collection.
  $collection = _csa_collection_get($coll_id, array('fatal' => $options['fatal']));

  $request_options = array(
    'user_id' => $user_id,
    'fatal' => $options['fatal'],
    'data' => array(
      'asset_id' => $asset_ids,
    ),
  );

  // Setup logging.
  $message = "Unable to remove assets from collection '@name'";
  $variables = array('@name' => _csa_collection_get_title($collection));

  try {
    // Set acl rights.
    $response = _csa_mediamosa_request_post('collection/' . rawurlencode($coll_id) . '/asset_relation/delete', $request_options);
  }
  catch (Exception $e) {
    _csa_log_rest_response(FALSE, $message, $variables);
    throw $e;
  }

  // Log it (if the request failed).
  $result = _csa_log_rest_response($response, $message, $variables);

  // Return the number of assets really removed.
  if ($result) {
    return count($response->xml->items->item);
  }

  return FALSE;
}

/**
 * Select collection.
 *
 * @param array $form_state
 * @param string $type
 */
function _csa_collection_select_form($form_state, $type, $description = '') {

  // Get collections.
  $collections = _csa_collection_search(array('for_select' => TRUE));

  $form['collection'] = array(
    '#type' => 'fieldset',
    '#title' => t('Select collection'),
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
    '#description' => $description,
  );

  $form['collection']['coll_id'] = array(
    '#type' => 'select',
    '#options' => $collections,
    '#required' => TRUE,
    '#size' => 1,
  );

  $form['submit']['select_collection'] = array(
    '#type' => 'submit',
    '#value' => t(CSA_SUBMIT_TEXT_COLLECTION_SELECT),
  );

  $form['submit']['cancel'] = array(
    '#type' => 'button',
    '#value' => t(CSA_SUBMIT_TEXT_COLLECTION_SELECT_CANCEL),
  );

  $form['type'] = array(
    '#type' => 'hidden',
    '#value' => $type,
  );

  return $form;
}

/**
 * Save collection properties.
 *
 * @param string $coll_id
 * @param array $properties
 */
function _csa_collection_save($coll_id, $properties, $options = array()) {

  $options += array(
    'is_app_admin' => TRUE, // no need for owner id.
    'fatal' => FALSE,
    'set_message' => FALSE,
    'user_id' => _csa_get_mediamosa_user_id(),
  );

  $options['is_app_admin'] = $options['is_app_admin'] ? 'TRUE' : 'FALSE';

  // Set the properties.
  foreach ($properties as $name => $value) {
    $options[$name] = $value;
  }

  $request_options = array(
    'data' => $options,
    'fatal' => $options['fatal'],
  );

  // Unset these.
  unset($request_options['data']['fatal'], $request_options['data']['set_message']);

  // Setup logging.
  $message = 'Unable to save collection with ID @coll_id';
  $variables = array('@coll_id' => $coll_id);

  try {
    // Do REST call.
    $response = _csa_mediamosa_request_post('collection/' . rawurlencode($coll_id), $request_options);
  }
  catch (Exception $e) {
    _csa_log_rest_response(FALSE, $message, $variables);
    throw $e;
  }

  // Set message.
  if (mediamosa_connector::check_result($response)) {
    if (!empty($options['set_message'])) {
      $collection = _csa_collection_get($coll_id);
      drupal_set_message(t("Collection '@title' has been saved.", array('@title' => _csa_collection_get_title($collection))));
    }
  }

  // Log it (if the request failed).
  return _csa_log_rest_response($response, $message, $variables);
}

/**
 * Delete a collection.
 *
 * @param $coll_id
 *   The collection ID.
 * @param $options
 *   Optional settings.
 */
function _csa_collection_delete($coll_id, $user_id, $options = array()) {

  $options += array(
    'fatal' => FALSE,
    'cascade' => '',
    'set_message' => FALSE,
  );

  // Just check.
  assert(in_array($options['delete'], array('', 'cascade')));

  $request_options = array(
    'user_id' => $user_id,
    'fatal' => $options['fatal'],
    'data' => array(
      'cascade' => $options['cascade'],
    ),
  );

  // Setup logging.
  $message = 'Unable to delete collection with ID @coll_id';
  $variables = array('@coll_id' => $coll_id);

  try {
    // Do REST call.
    $response = _csa_mediamosa_request_post('collection/' . rawurlencode($coll_id) . '/delete', $request_options);
  }
  catch (Exception $e) {
    _csa_log_rest_response(FALSE, $message, $variables);
    throw $e;
  }

  // Set message.
  if (mediamosa_connector::check_result($response)) {
    if (!empty($options['set_message'])) {
      $collection = _csa_collection_get($coll_id);
      drupal_set_message(t("Collection '@title' has been deleted.", array('@title' => _csa_collection_get_title($collection))));
    }
  }

  // Log it (if the request failed).
  return _csa_log_rest_response($response, $message, $variables);
}

/**
 * Publish collection.
 *
 * @param string $coll_id
 * @param array $properties
 */
function _csa_collection_publish($coll_id, $user_id, $options = array()) {
  global $user;

  $options += array(
    'fatal' => FALSE,
    'set_message' => FALSE,
  );

  $request_options = array(
    'user_id' => $user_id,
    'fatal' => $options['fatal'],
    'data' => array(
      'acl_app' => array_keys($user->csa_access_rules['applications']),
    ),
  );

  // Setup logging.
  $message = 'Unable to publish collection with ID @coll_id';
  $variables = array('@coll_id' => $coll_id);

  try {
    // Do REST call.
    $response = _csa_mediamosa_request_post('collection/' . rawurlencode($coll_id) . '/acl', $request_options);
  }
  catch (Exception $e) {
    _csa_log_rest_response(FALSE, $message, $variables);
    throw $e;
  }

  // Set message.
  if (mediamosa_connector::check_result($response)) {
    if (!empty($options['set_message'])) {
      $collection = _csa_collection_get($coll_id);
      drupal_set_message(t("Collection '@title' has been published.", array('@title' => _csa_collection_get_title($collection))));
    }
  }
}

/**
 * Unpublish collection.
 *
 * @param string $coll_id
 * @param array $properties
 */
function _csa_collection_unpublish($coll_id, $user_id, $options = array()) {
  $options += array(
    'fatal' => FALSE,
    'set_message' => FALSE,
  );

  $request_options = array(
    'user_id' => $user_id,
    'fatal' => $options['fatal'],
    'data' => array(
      'acl_app' => array(),
    ),
  );

  // Setup logging.
  $message = 'Unable to unpublish collection with ID @coll_id';
  $variables = array('@coll_id' => $coll_id);

  try {
    // Do REST call.
    $response = _csa_mediamosa_request_post('collection/' . rawurlencode($coll_id) . '/acl', $request_options);
  }
  catch (Exception $e) {
    _csa_log_rest_response(FALSE, $message, $variables);
    throw $e;
  }

  // Set message.
  if (mediamosa_connector::check_result($response)) {
    if (!empty($options['set_message'])) {
      $collection = _csa_collection_get($coll_id);
      drupal_set_message(t("Collection '@title' has been unpublished.", array('@title' => _csa_collection_get_title($collection))));
    }
  }
}
