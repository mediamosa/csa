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

// Submit buttons.
define('CSA_COLLECTION_SAVE', 'Save');
define('CSA_COLLECTION_SAVE_AND_DONE', 'Save & done');
define('CSA_COLLECTION_SAVE_AND_NEXT', 'Save & next collection');
define('CSA_COLLECTION_CANCEL', 'Cancel');
define('CSA_COLLECTION_PUBLISH', 'Publish');
define('CSA_COLLECTION_UNPUBLISH', 'Unpublish');
define('CSA_COLLECTION_PUBLISH_AND_NEXT', 'Publish & next collection');
define('CSA_COLLECTION_UNPUBLISH_AND_NEXT', 'Unpublish & next collection');

/**
 * @file
 * The collection create, edit and delete functions.
 */

// Number of assets per page under collection. @todo: into settings of admin.
define('CSA_COLLECTIONS_PER_PAGE', 50);
define('CSA_COLLECTION_ASSETS_PER_PAGE', 50);


/**
 * Retrieves the listing for the collections.
 *
 * @param array $options
 */
function _csa_collection_get_collection_listing($options) {

  $options += array(
    'limit' => CSA_COLLECTIONS_PER_PAGE,
    'offset' => 0,
    'calculate_total_count' => TRUE,
    'order_by' => 'title',
    'order_direction' => 'ASC',
    'group_ids' => array(),
    'for_select' => FALSE,
  );

  // Get the collections.
  return _csa_collection_search($options);
}

/**
 * Show a list of the collections.
 *
 * @param array $form_state
 */
function _csa_collection_list() {

  $form = array();

  // Basic stuff.
  $page = isset($_GET['page']) ? $_GET['page'] : 0;
  $limit = CSA_COLLECTIONS_PER_PAGE;
  $offset = $page * $limit;

  // Get the collections.
  $collections = _csa_collection_get_collection_listing(array('limit' => $limit, 'offset' => $offset));

  // Setup for theme function.
  $options['page'] = $page;
  $options['item_count'] = $collections['item_count_total'];
  $options['item_limit'] = $limit;
  $options['empty'] = t('No collections were found.');
  $options['offset'] = $offset;

  // List the collections.
  $output['collections'] = theme('csa_collection_list', $collections['collections'], $options);

  return implode('', $output);
}

/**
 * Add a new collection.
 */
function _csa_collection_add() {
  // Output form.
  $output['collection_form'] = drupal_get_form('_csa_collection_form_add');

  return implode('', $output);
}

/**
 * The collection add form.
 */
function _csa_collection_form_add($form_state) {
  return _csa_collection_form($form_state);
}

/**
 * Validate the form submit.
 */
function _csa_collection_form_add_validate() {
}

/**
 * Process the submit.
 */
function _csa_collection_form_add_submit($form, &$form_state) {

  $values = $form_state['values'];
  switch ($values['op']) {
    case t(CSA_COLLECTION_SAVE):

      $options = array(
        'title' => $values['title'],
        'description' => $values['description'],
      );

      // Create the collection.
      $coll_id = _csa_collection_create(_csa_get_mediamosa_user_id(), $options);

      // Collection made? Go to the detail page.
      if (!empty($coll_id)) {
        drupal_goto(strtr('csa/collection/!coll_id/!offset', array('!coll_id' => $coll_id, '!offset' => rawurlencode('-1'))));
      }
      break;
  }
}


/**
 * Show the details of a collection.
 */
function _csa_collection_detail($coll_id, $offset, $options = array()) {

  $options += array(
  );

  // Get the collection.
  $collection = _csa_collection_get($coll_id);

  // Get the title.
  $title = _csa_collection_get_title($collection);

  // Set asset title.
  drupal_set_title($title);

  $output = array();

  // Need to cache this for its globals its sets during the function.
  theme('csa_collection_previous_next_collection', 'csa/collection/!coll_id/!offset', $offset);

  // Properties.
  $output['properties'] = theme('csa_collection', $collection, $offset);

  // Settings for the asset listing pager.
  $asset_page = isset($_GET['page']) ? $_GET['page'] : 0;
  $asset_limit = CSA_COLLECTION_ASSETS_PER_PAGE;
  $asset_offset = $asset_page * $asset_limit;

  // Get the assets.
  $options_assets = array(
    'coll_id' => array($coll_id),
    'limit' => $asset_limit,
    'offset' => $asset_offset,
    'hide_empty_assets' => TRUE,
    'return_asset_ids' => TRUE,
    'order_by' => 'videotimestamp',
  );
  $asset_ids = _csa_asset_search($options_assets);

  // Now get the mediafile info.
  $assets = _csa_assets_get(array_keys($asset_ids['assets']));

  // Options theme assets.
  $options['page'] = $options_assets['page'];
  $options['item_count'] = $asset_ids['item_count_total'];
  $options['item_limit'] = $options_assets['limit'];
  $options['offset'] = $options_assets['offset'];

  // Assets in collection.
  $output['assets'] = theme('csa_collection_assets', $collection, $assets, $options);

  return implode('<br />', $output);
}

/**
 * Edit form for collection.
 *
 * @param string $coll_id
 * @param integer $offset
 * @param array $options
 */
function _csa_collection_edit($coll_id, $offset, $options = array()) {

  // Get the asset.
  $collection = _csa_collection_get($coll_id);

  // Get the title.
  $title = _csa_collection_get_title($collection);

  // Set asset title.
  drupal_set_title($title);

  // Will pregenerate and cache, need to do it now because we need the globals.
  theme('csa_collection_previous_next_collection', 'csa/collection/!coll_id/!offset/edit', $offset);

  // Output form.
  $output['collection_form'] = drupal_get_form('_csa_collection_form', $coll_id, $offset);

  return implode('', $output);
}

/**
 * The collection add / edit form.
 *
 * @param $form_state
 * @param $coll_id
 * @param $options
 */
function _csa_collection_form($form_state, $coll_id = NULL, $offset = -1, $options = array()) {

  if (!empty($coll_id)) {
    // Get the collection.
    $collection = _csa_collection_get($coll_id);
  }

  // Setup defaults.
  $options += array(
    'button_save_and_done' => empty($_POST['button_save_and_done']) ? TRUE : $_POST['button_save_and_done'],
    'button_cancel' => empty($_POST['button_cancel']) ? TRUE : $_POST['button_cancel'],
    'default_destination' => 'csa/collection',
  );

  // The properties fieldset.
  $form['properties'] = array(
    '#type' => 'fieldset',
    '#title' => t('Properties'),
    '#weight' => -5,
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
  );

  $form['properties']['title'] = array(
    '#type' => 'textfield',
    '#title' => t('Title'),
    '#description' => t('Title of collection.'),
    '#maxlength' => 255,
    '#required' => TRUE,
    '#default_value' => empty($collection['title']) ? '' : $collection['title'],
  );
  $form['properties']['description'] = array(
    '#type' => 'textarea',
    '#title' => t('Description'),
    '#description' => t('Short description of the collection.'),
    '#maxlength' => 1000,
    '#default_value' => empty($collection['description']) ? '' : $collection['description'],
  );

  $form['coll_id'] = array(
    '#type' => 'hidden',
    '#value' => $coll_id,
  );

  $form['offset'] = array(
    '#type' => 'hidden',
    '#value' => $offset,
  );
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t(CSA_COLLECTION_SAVE),
  );

  // Submit buttons
  if (!empty($GLOBALS['collection_next_url'])) {
    $form['collection_next_url'] = array(
      '#type' => 'hidden',
      '#value' => $GLOBALS['collection_next_url'],
    );

    $form['buttons']['save_next'] = array(
      '#type' => 'submit',
      '#value' => t(CSA_COLLECTION_SAVE_AND_NEXT),
    );
  }

  if (!empty($options['button_cancel'])) {
    $form['buttons']['cancel'] = array(
      '#type' => 'submit',
      '#value' => t(CSA_COLLECTION_CANCEL),
    );
  }

  // Save the contents of options.
  foreach ($options as $name => $value) {
    if (!empty($value)) {
      $form['options'][$name] = array(
        '#type' => 'hidden',
        '#value' => $value,
      );
    }
  }

  return $form;
}

/**
 * Validate function for collection form.
 */
function _csa_collection_form_validate($form, &$form_state) {

  $values = $form_state['values'];
  switch ($values['op']) {
    case CSA_COLLECTION_CANCEL:
      form_set_error(NULL, '', TRUE);
      drupal_get_messages('error', TRUE);

      // Build our destination from scratch, redirect to view page.
      // Don't forget offset.
      $_REQUEST['destination'] = strtr('csa/collection/!coll_id/!offset', array('!coll_id' => rawurlencode($values['coll_id']), '!offset' => $values['offset']));
      drupal_goto();
      break;
  }
}

/**
 * Submit function for collection form.
 */
function _csa_collection_form_submit($form, &$form_state) {

  $values = $form_state['values'];
  switch ($values['op']) {
    case t(CSA_COLLECTION_SAVE_AND_NEXT):
    case t(CSA_COLLECTION_SAVE):
      unset($form_state['storage']);
      $properties = array();
      foreach (array('title', 'description') as $name) {
        $properties[$name] = $values[$name];
      }

      // Save it.
      _csa_collection_save($values['coll_id'], $properties, array('set_message' => TRUE));

      // Log it.
      _csa_log_info('Saved collection with ID @coll_id', array('@coll_id' => $values['coll_id']), url('csa/collection/' . $values['coll_id']));
      break;
  }

  switch ($values['op']) {
    case t(CSA_COLLECTION_SAVE_AND_NEXT):
      $_REQUEST['destination'] = $values['collection_next_url'];
      break;

    case t(CSA_COLLECTION_SAVE):
    default:
      $_REQUEST['destination'] = strtr('csa/collection/!coll_id/!offset', array('!coll_id' => rawurlencode($values['coll_id']), '!offset' => $values['offset']));
      break;
  }
}

/**
 * (un)Publish form for collection.
 *
 * @param string $coll_id
 * @param integer $offset
 * @param array $options
 */
function _csa_collection_publication($coll_id, $offset, $options = array()) {

  // Get the asset.
  $collection = _csa_collection_get($coll_id);

  // Get the title.
  $title = _csa_collection_get_title($collection);

  // Set asset title.
  drupal_set_title($title);

  // Will pregenerate and cache, need to do it now because we need the globals.
  theme('csa_collection_previous_next_collection', 'csa/collection/!coll_id/!offset/publication', $offset);

  // Output form.
  $output['collection_publication_form'] = drupal_get_form('_csa_collection_publication_form', $coll_id, $offset);

  return implode('', $output);
}

/**
 * The collection (un)publish form.
 *
 * @param $form_state
 * @param $coll_id
 * @param $options
 */
function _csa_collection_publication_form($form_state, $coll_id, $offset = -1, $options = array()) {
  global $user;

  // Get the collection.
  $collection = _csa_collection_get($coll_id);

  // Get publish status.
  $is_published = _csa_collection_is_published($coll_id, $collection['owner_id']);

  // Setup defaults.
  $options += array(
    'button_save_and_done' => empty($_POST['button_save_and_done']) ? TRUE : $_POST['button_save_and_done'],
    'button_cancel' => empty($_POST['button_cancel']) ? TRUE : $_POST['button_cancel'],
    'default_destination' => 'csa/collection/publication',
  );

  // Allowed by default.
  $publication_allowed = TRUE;

  if ($is_published) {
    $description = t("The collection '@coll_name' is published, press the unpublish button bellow to remove the publication status.", array('@coll_name' => $collection['title']));
    $submit = t(CSA_COLLECTION_UNPUBLISH);
    $submit_next = t(CSA_COLLECTION_UNPUBLISH_AND_NEXT);
  }
  else {
    $description = t("The collection '@coll_name' is not published, press the publish button bellow to publish the collection.", array('@coll_name' => $collection['title']));
    $submit = t(CSA_COLLECTION_PUBLISH);
    $submit_next = t(CSA_COLLECTION_PUBLISH_AND_NEXT);

    // Check if the publish is setup.
    if (empty($user->csa_access_rules['applications'])) {
      drupal_set_message(t("There are no publication applications specified, publication is not possible. Add applications !here under section 'Access rules'.", array('!here' => l(t('here'), 'csa/prefs', array('query' => array('csa_prefs_op' => CSA_SUBMIT_TEXT_PREFS_ADD_DOMAIN_REALM))))), 'warning');
      $description = t("The collection '@coll_name' is not published.", array('@coll_name' => $collection['title']));

      $publication_allowed = FALSE;
    }
  }

  // The publication fieldset.
  $form['publication'] = array(
    '#type' => 'fieldset',
    '#title' => t('Publication'),
    '#description' => $description,
    '#weight' => -5,
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
  );

  $form['coll_id'] = array(
    '#type' => 'value',
    '#value' => $coll_id,
  );
  $form['user_id'] = array(
    '#type' => 'value',
    '#value' => $collection['owner_id'],
  );

  $form['offset'] = array(
    '#type' => 'value',
    '#value' => $offset,
  );

  if ($publication_allowed) {
    $form['submit'] = array(
      '#type' => 'submit',
      '#value' => $submit,
    );

    // Submit buttons
    if (!empty($GLOBALS['collection_next_url'])) {
      $form['collection_next_url'] = array(
        '#type' => 'value',
        '#value' => $GLOBALS['collection_next_url'],
      );

      $form['buttons']['publish_next'] = array(
        '#type' => 'submit',
        '#value' => $submit_next,
      );
    }
  }

  if (!empty($options['button_cancel'])) {
    $form['buttons']['cancel'] = array(
      '#type' => 'submit',
      '#value' => t(CSA_COLLECTION_CANCEL),
    );
  }

  // Save the contents of options.
  foreach ($options as $name => $value) {
    if (!empty($value)) {
      $form['options'][$name] = array(
        '#type' => 'value',
        '#value' => $value,
      );
    }
  }

  return $form;
}

/**
 * Validate function for publish collection form.
 */
function _csa_collection_publication_form_validate($form, &$form_state) {

  $values = $form_state['values'];
  switch ($values['op']) {
    case CSA_COLLECTION_CANCEL:
      form_set_error(NULL, '', TRUE);
      drupal_get_messages('error', TRUE);

      // Build our destination from scratch, redirect to view page.
      // Don't forget offset.
      $_REQUEST['destination'] = strtr('csa/collection/!coll_id/!offset', array('!coll_id' => rawurlencode($values['coll_id']), '!offset' => $values['offset']));
      drupal_goto();
      break;
  }
}

/**
 * Submit function for collection form.
 */
function _csa_collection_publication_form_submit($form, &$form_state) {

  $values = $form_state['values'];
  switch ($values['op']) {
    case t(CSA_COLLECTION_PUBLISH_AND_NEXT):
    case t(CSA_COLLECTION_PUBLISH):
      // Publish it.
      _csa_collection_publish($values['coll_id'], $values['user_id'], array('set_message' => TRUE));

      // Log it.
      _csa_log_info('Published collection with ID @coll_id', array('@coll_id' => $values['coll_id']), url('csa/collection/' . $values['coll_id']));
      break;

    case t(CSA_COLLECTION_UNPUBLISH_AND_NEXT):
    case t(CSA_COLLECTION_UNPUBLISH):
      // Publish it.
      _csa_collection_unpublish($values['coll_id'], $values['user_id'], array('set_message' => TRUE));

      // Log it.
      _csa_log_info('Published collection with ID @coll_id', array('@coll_id' => $values['coll_id']), url('csa/collection/' . $values['coll_id']));
      break;
  }

  switch ($values['op']) {
    case t(CSA_COLLECTION_PUBLISH_AND_NEXT):
    case t(CSA_COLLECTION_UNPUBLISH_AND_NEXT):
      $_REQUEST['destination'] = $values['collection_next_url'];
      break;

    case t(CSA_COLLECTION_PUBLISH):
    case t(CSA_COLLECTION_UNPUBLISH):
    default:
      $_REQUEST['destination'] = strtr('csa/collection/!coll_id/!offset/publication', array('!coll_id' => rawurlencode($values['coll_id']), '!offset' => $values['offset']));
      break;
  }
}

/**
 * Delete the collection confirm dialog.
 *
 * @param string $type
 */
function _csa_collection_delete_form($form_state, $coll_id, $offset) {
  $form['coll_id'] = array('#type' => 'hidden', '#value' => $coll_id);
  $form['offset'] = array('#type' => 'hidden', '#value' => $offset);

  // Get the collection.
  $collection = _csa_collection_get($coll_id);

  return confirm_form($form,
                      t('Are you sure you want to delete the collection %name?', array('%name' => $collection['title'])),
                      strtr('csa/collection/!coll_id/!offset', array('!coll_id' => rawurlencode($coll_id), '!offset' => $offset)),
                      t('This action cannot be undone.'),
                      t('Delete collection'),
                      t('Cancel')
  );
}

/**
 * Delete selected items.
 *
 * @param $feed
 *   An associative array describing the feed to be cleared.
 */
function _csa_collection_delete_form_submit($form, &$form_state) {

  // Get the values.
  $coll_id = $form_state['values']['coll_id'];
  $offset = $form_state['values']['offset'];

  switch ($form_state['values']['op']) {
    case t('Delete collection'):

      // Get collection.
      $collection = _csa_collection_get($coll_id);

      if (!$collection) {
        drupal_set_message('Unable to delete collection, unable to find the collection.', 'error');
        drupal_goto('csa/collection');
      }

      // This will cache the next / previous link I need.
      theme('csa_collection_previous_next_collection', 'csa/collection/!coll_id/!offset', $offset);

      if (!_csa_collection_delete($collection['coll_id'], $collection['owner_id'], array('set_message' => TRUE))) {
        // Failed, go to details.
        drupal_goto(strtr('csa/collection/!coll_id/!offset', array('!coll_id' => rawurlencode($coll_id), '!offset' => $form_state['values']['offset'])));
      }

      // Deleted. Let go to next.
      if (!empty($GLOBALS['collection_next_id'])) {
        // Next it is, offset should be the same.
        drupal_goto(strtr('csa/collection/!coll_id/!offset', array('!coll_id' => rawurlencode($GLOBALS['collection_next_id']), '!offset' => $form_state['values']['offset'])));
      }
      elseif (!empty($GLOBALS['collection_previous_id'])) {
        drupal_goto(strtr('csa/collection/!coll_id/!offset', array('!coll_id' => rawurlencode($GLOBALS['collection_previous_id']), '!offset' => $form_state['values']['offset'] - 1)));
      }

      // Hmm ok all collections gone, go to listing then.
      drupal_goto('csa/collection');
      break;

    default:
    case t('Cancel'):
      drupal_set_message('You have canceled the deletion.');
      drupal_goto(strtr('csa/collection/!coll_id/!offset', array('!coll_id' => rawurlencode($coll_id), '!offset' => $form_state['values']['offset'])));
  }
}

/**
 * Select the collection in the search form and jump to search page.
 */
function _csa_collection_select_for_search($coll_id) {
  // Clean form first.
  _csa_box_search_clean_session();

  // Select the collection in the search form.
  $_SESSION['csa_forms'][CSA_BOX_TYPE_SEARCH]['values']['search_collection'] = $coll_id;

  // Goto to search page.
  drupal_goto('csa/search', 'do_search=1');
}
