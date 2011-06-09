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
 * This include contains all code related to helper functions for mediafiles.
 */

// Submit text buttons.
define('CSA_SUBMIT_TEXT_ASSET_MEDIAFILE_START_TRANSCODINGS', 'Start transcoding');
define('CSA_SUBMIT_TEXT_ASSET_MEDIAFILE_CANCEL', 'Cancel');

define('CSA_SUBMIT_TEXT_ASSET_MEDIAFILE_START_ACCESS', 'Start batch');
define('CSA_SUBMIT_TEXT_ASSET_MEDIAFILE_CANCEL_ACCESS', 'Cancel');

define('CSA_SUBMIT_TEXT_ASSET_MEDIAFILE_SUBMIT_ACCESS', 'Save');

define('CSA_TRANSCODE_REPLACE_OPTION_ADD', 'ADD');
define('CSA_TRANSCODE_REPLACE_OPTION_REPLACE', 'REPLACE');
define('CSA_TRANSCODE_REPLACE_OPTION_SKIP', 'SKIP');

/**
 * Get the asset.
 *
 * @param $mediafile_id
 *  The mediafile ID.
 * @param $options
 */
function _csa_asset_mediafile_get($mediafile_id, $options = array()) {

  static $mediafiles = array();

  $options += array(
    'show_stills' => TRUE,
    'fatal' => FALSE,
  );

  if (!empty($mediafiles[$mediafile_id][$options['show_stills']])) {
    return $mediafiles[$mediafile_id][$options['show_stills']];
  }

  $request_options = array(
    'data' => array(
      'show_stills' => $options['show_stills'] ? 'TRUE' : 'FALSE',
    ),
    'fatal' => $options['fatal'],
  );

  // Get the mediafile.
  $mediafiles[$mediafile_id][$options['show_stills']] = mediamosa_connector::response2array(_csa_mediamosa_request('mediafile/' . $mediafile_id, $request_options));
  return $mediafiles[$mediafile_id][$options['show_stills']];
}

/**
 * Get the ACL of mediafile.
 *
 * @param string $mediafile_id
 * @param string $user_id
 */
function _csa_asset_mediafile_get_acl($mediafile_id, $user_id) {

  // Setup logging.
  $message = 'Unable to get ACL from mediafile with ID @mediafile_id';
  $variables = array('@mediafile_id' => $mediafile_id);

  $options = array(
    'user_id' => $user_id,
  );

  try {
    // Set acl rights.
    $response = _csa_mediamosa_request('mediafile/' . rawurlencode($mediafile_id) . '/acl', $options);
  }
  catch (Exception $e) {
    _csa_log_rest_response(FALSE, $message, $variables);
    throw $e;
  }

  // Log it (if the request failed).
  _csa_log_rest_response($response, $message, $variables);

  // Get the response.
  $response = mediamosa_connector::response2array($response);

  return empty($response['items']['item']) ? array() : $response['items']['item'];
}

/**
 * Set ACL rights on mediafile.
 *
 * @param string $mediafile_id
 * @param string $user_id
 * @param array $options
 *  - aut_app
 *  - aut_domain
 *  - aut_realm
 *  - aut_group
 *  - aut_user
 *  - replace (default TRUE)
 *
 *  @see REST call /mediafile/$asset_id/acl
 */
function _csa_asset_mediafile_set_acl($mediafile_id, $user_id, $options = array()) {

  // Set default.
  $options += array(
    'replace' => TRUE,
  );

  $data = array();
  foreach (array('aut_app', 'aut_domain', 'aut_realm', 'aut_user', 'aut_group') as $subject) {
    if (!empty($options[$subject])) {
      $data[$subject] = $options[$subject];
    }
  }

  if (empty($data)) {
    throw new Exception('No access rules set for _csa_asset_mediafile_set_acl()');
  }

  // Set replace flag.
  $data['replace'] = ($options['replace'] ? 'TRUE' : 'FALSE');

  $options = array(
    'user_id' => $user_id,
    'data' => $data,
  );

  // Setup logging.
  $message = 'Unable to set ACL on mediafile with ID @mediafile_id';
  $variables = array('@mediafile_id' => $mediafile_id);

  try {
    // Set acl rights.
    $response = _csa_mediamosa_request_post('mediafile/' . rawurlencode($mediafile_id) . '/acl', $options);
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
 */
function _csa_asset_mediafile_delete_acl($mediafile_id, $user_id) {

  $options = array(
    'user_id' => $user_id,
  );

  // Setup logging.
  $message = 'Unable to remove ACL rights from mediafile with ID @mediafile_id';
  $variables = array('@mediafile_id' => $mediafile_id);

  try {
    // Do REST call.
    $response = _csa_mediamosa_request_post('mediafile/' . rawurlencode($mediafile_id) . '/acl/delete', $options);
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
 */
function _csa_asset_mediafile_delete($mediafile_id, $user_id, $options = array()) {

  // Set user_id.
  $options['user_id'] = $user_id;

  $options += array(
    'delete' => 'cascade',
  );

  // Setup logging.
  $message = 'Unable to delete mediafile with ID @mediafile_id';
  $variables = array('@mediafile_id' => $mediafile_id);

  try {
    // Do REST call.
    $response = _csa_mediamosa_request_post('mediafile/' . rawurlencode($mediafile_id) . '/delete', $options);
  }
  catch (Exception $e) {
    _csa_log_rest_response(FALSE, $message, $variables);
    throw $e;
  }

  // Log it (if the request failed).
  return _csa_log_rest_response($response, $message, $variables);
}

/**
 * Transcode mediafile.
 *
 * @param string $mediafile_id
 *   Must be original.
 * @param string $user_id
 *   Owner of mediafile.
 * @param string $profile_id
 *   The profile ID.
 */
function _csa_asset_mediafile_transcode($mediafile_id, $user_id, $profile_id, $options = array()) {

  // Set up options.
  $options['profile_id'] = $profile_id;

  // Default.
  $options += array(
    'completed_transcoding_url' => '',
  );

  // Set options for REST call.
  $options = array('data' => $options);
  $options['user_id'] = $user_id;

  // Setup logging.
  $message = 'Unable to start transcode for mediafile with ID @mediafile_id';
  $variables = array('@mediafile_id' => $mediafile_id);

  try {
    // Do REST call.
    $response = _csa_mediamosa_request_post('mediafile/' . rawurlencode($mediafile_id) . '/transcode', $options);
  }
  catch (Exception $e) {
    _csa_log_rest_response(FALSE, $message, $variables);
    throw $e;
  }

  // Log it (if the request failed).
  return _csa_log_rest_response($response, $message, $variables);
}


/**
 * Build a basic form for transcode selection.
 */
function _csa_asset_mediafile_transcode_select_form($form_state, $type) {
  global $user;

  // Get the transcode list.
  $profile_ids = _csa_get_transcodings($type);

  // Options for replace.
  $transcode_replace = array(
    CSA_TRANSCODE_REPLACE_OPTION_REPLACE => t('Replace existing transcodings with the same transcode profile'),
    CSA_TRANSCODE_REPLACE_OPTION_ADD => t('Always add new transcodings'),
    CSA_TRANSCODE_REPLACE_OPTION_SKIP => t('Skip transcode when transcode exists with the same transcode profile')
  );

  $form['transcode'] = array(
    '#type' => 'fieldset',
    '#title' => t('Select transcode'),
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
    '#description' => t("Select one or more transcode profiles for your batch."),
  );

  $form['transcode']['profile_ids'] = array(
    '#type' => 'select',
    '#multiple' => TRUE,
    '#size' => count($profile_ids) < 10 ? count($profile_ids) : 10,
    '#options' => $profile_ids,
    '#required' => TRUE,
  );

  $form['transcode']['transcode_replace'] = array(
    '#type' => 'select',
    '#title' => t('How to handle existing transcodings with same transcode profile'),
    '#options' => $transcode_replace,
    '#default_value' => CSA_TRANSCODE_REPLACE_OPTION_REPLACE,
    '#description' => t("Select how to handle new transcodings in your batch when an asset already has transcodings of the chosen profile. Select 'Add new transcodings' when you don't mind creating double transcodings with the same profile. Use 'Replace' option to replace existing transcodings with the same profile ID. The last option 'Skip transcoding' enables you to skip the transcode when there is already exists a transcode of the asset with the chosen profile."),
  );

  $form['submit']['select_transcodings'] = array(
    '#type' => 'submit',
    '#value' => t(CSA_SUBMIT_TEXT_ASSET_MEDIAFILE_START_TRANSCODINGS),
  );

  $form['submit']['cancel'] = array(
    '#type' => 'button',
    '#value' => t(CSA_SUBMIT_TEXT_ASSET_MEDIAFILE_CANCEL),
  );

  $form['type'] = array(
    '#type' => 'hidden',
    '#value' => $type,
  );

  return $form;
}

/**
 * Display the access rules form.
 */
function _csa_asset_mediafile_access_select_form($form_state, $type = 0) {

  $select_access_rules = array();
  if (!empty($form_state['storage']['access']['domains'])) {
    foreach ($form_state['storage']['access']['domains'] as $domain) {
      $select_access_rules[t(CSA_ACCESS_TYPE_DOMAIN) . $domain] = t(CSA_ACCESS_TYPE_DOMAIN) . $domain;
    }
  }
  if (!empty($form_state['storage']['access']['realms'])) {
    foreach ($form_state['storage']['access']['realms'] as $realm) {
      $select_access_rules[t(CSA_ACCESS_TYPE_REALM) . $realm] = t(CSA_ACCESS_TYPE_REALM) . $realm;
    }
  }
  if (!empty($form_state['storage']['access']['applications'])) {
    foreach ($form_state['storage']['access']['applications'] as $id => $application) {
      $select_access_rules[$id] = t(CSA_ACCESS_TYPE_APP) . $application;
    }
  }

  $form['access'] = array(
    '#type' => 'fieldset',
    '#title' => t('Access rules'),
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
  );

  $form['access']['access_input'] = array(
    '#prefix' => '<div class="csa-prefs-input">',
    '#suffix' => '</div>',
  );

  /*
  $form['access']['access_input']['fieldset_2'] = array(
    '#type' => 'fieldset',
    '#title' => t('Available applications'),
    '#collapsible' => FALSE,
    '#collapsed' => FALSE,
  );
  $form['access']['access_input']['fieldset_2']['application'] = array(
    '#type' => 'select',
    '#options' => $applications,
    '#size' => 5,
    '#multiple' => TRUE,
    '#attributes' => array('class' => 'csa-select-prefs'),
  );
  $form['access']['access_input']['fieldset_2']['application_submit'] = array(
    '#type' => 'submit',
    '#value' => t(CSA_SUBMIT_TEXT_PREFS_ADD_APP),// Fixed OP text, change op switch too.
  );
  */

  $form['access']['access_input']['fieldset_1'] = array(
    '#type' => 'fieldset',
    '#title' => t('Domain / Realm'),
    '#collapsible' => FALSE,
    '#collapsed' => FALSE,
  );
  $form['access']['access_input']['fieldset_1']['domain_realm'] = array(
    '#type' => 'textfield',
    '#default_value' => '',
    '#size' => 40,
    '#maxlength' => 255,
  );
  $form['access']['access_input']['fieldset_1']['domain_realm_submit'] = array(
    '#type' => 'submit',
    '#value' => t(CSA_SUBMIT_TEXT_PREFS_ADD_DOMAIN_REALM),// Fixed OP text, change op switch too.
  );

  $form['access']['access_output'] = array(
    '#type' => 'fieldset',
    '#title' => t('Selected access rules'),
    '#collapsible' => FALSE,
    '#collapsed' => FALSE,
    '#prefix' => '<div class="csa-prefs-output">',
    '#suffix' => '</div>',
  );
  $form['access']['access_output']['selected_access_rules'] = array(
    '#type' => 'select',
    '#options' => $select_access_rules,
    '#size' => 10,
    '#multiple' => TRUE,
    '#attributes' => array('class' => 'csa-select-prefs'),
  );
  $form['access']['access_output']['selected_access_rules_remove'] = array(
    '#type' => 'submit',
    '#value' => t(CSA_SUBMIT_TEXT_PREFS_REMOVE_ACCESS_RULE),// Fixed OP text, change op switch too.
  );

  $form['#validate'][] = '_csa_asset_mediafile_access_select_form_validate';
  $form['#submit'][] = '_csa_asset_mediafile_access_select_form_submit';

  $form['type'] = array(
    '#type' => 'hidden',
    '#value' => $type,
  );

  return $form;
}

/**
 * Validate function for access form.
 */
function _csa_asset_mediafile_access_select_form_validate($form, $form_state) {
  // Get the values;
  $values = $form_state['values'];

  switch ($values['op']) {
    case t(CSA_SUBMIT_TEXT_PREFS_ADD_DOMAIN_REALM):
      // Get value.
      $value = $values['domain_realm'];

      if (empty($value)) {
        form_set_error('domain_realm', t('!name field is required.', array('!name' => t('Domain / Realm'))));
      }
      elseif (!_csa_is_realm($value) && !_csa_is_domain($value)) {
        form_set_error('domain_realm', t('Must be either a valid realm or domain.'));
      }
      break;

    case t(CSA_SUBMIT_TEXT_PREFS_ADD_APP):
      break;

    case t(CSA_SUBMIT_TEXT_PREFS_REMOVE_ACCESS_RULE):
      break;
  }
}

/**
 * Submit function for access form.
 */
function _csa_asset_mediafile_access_select_form_submit($form, &$form_state) {
  // Get the values;
  $values = $form_state['values'];

  switch ($values['op']) {
    case t(CSA_SUBMIT_TEXT_PREFS_ADD_DOMAIN_REALM):
      // Get value.
      $value = $values['domain_realm'];

      // Is realm or domain?
      if (_csa_is_realm($value)) {
        $form_state['storage']['access']['realms'][$value] = $value;

        // Show we added it.
        //drupal_set_message(t('Access rule for realm !realm added.', array('!realm' => $value)));

        // Prevent losing our posted data.
        $form_state['rebuild'] = TRUE;
      }
      elseif (_csa_is_domain($value)) {
        $form_state['storage']['access']['domains'][$value] = $value;

        // Show we added it.
        //drupal_set_message(t('Access rule for domain !domain added.', array('!domain' => $value)));

        // Prevent losing our posted data.
        $form_state['rebuild'] = TRUE;
      }
      break;
/*
    case t(CSA_SUBMIT_TEXT_PREFS_ADD_APP):
      // Get the applications.
      $applications = _csa_get_applications($account);

      // Go add them.
      foreach ($values['application'] as $value) {
        if (!empty($applications[$value])) {
          // Store it.
          $account->csa_access_rules['applications'][$value] = $applications[$value];

          // Show we added it.
          drupal_set_message(t('Access rule for application !application added.', array('!application' => $applications[$value])));
          $save_account = TRUE;
        }
      }
      break;
*/
    case t(CSA_SUBMIT_TEXT_PREFS_REMOVE_ACCESS_RULE):
      // Get the applications.
      //$applications = _csa_get_applications($account);

      foreach ($values['selected_access_rules'] as $to_remove) {
        if (substr($to_remove, 0, drupal_strlen(t(CSA_ACCESS_TYPE_DOMAIN))) == t(CSA_ACCESS_TYPE_DOMAIN)) {
          $value = substr($to_remove, drupal_strlen(CSA_ACCESS_TYPE_DOMAIN));

          // Remove.
          unset($form_state['storage']['access']['domains'][$value]);

          // Show we removed it.
          //drupal_set_message(t('Access rule for domain !domain removed.', array('!domain' => $value)));

          // Prevent losing our posted data.
          $form_state['rebuild'] = TRUE;
        }
        elseif (substr($to_remove, 0, drupal_strlen(t(CSA_ACCESS_TYPE_REALM))) == t(CSA_ACCESS_TYPE_REALM)) {
          $value = substr($to_remove, drupal_strlen(t(CSA_ACCESS_TYPE_REALM)));

          // Remove.
          unset($form_state['storage']['access']['realms'][$value]);

          // Show we removed it.
          //drupal_set_message(t('Access rule for realm !realm removed.', array('!realm' => $value)));

          // Prevent losing our posted data.
          $form_state['rebuild'] = TRUE;
        }
        /*
        elseif (is_numeric($to_remove) && !empty($account->csa_access_rules['applications'][$to_remove])) {

          // Need the name in case application no longer exists.
          $name = (empty($applications[$to_remove]) ? $account->csa_access_rules['applications'][$to_remove] : $applications[$to_remove]);

          // Remove.
          unset($account->csa_access_rules['applications'][$to_remove]);

          // Show we removed it.
          drupal_set_message(t('Access rule for application !application removed.', array('!application' => $name)));
          $save_account = TRUE;
        }*/
      }
      break;
  }
}

/**
 * Remove after confirm mediafile.
 *
 * @param string $mediafile_id
 */
function _csa_asset_mediafile_delete_form($form_state, $mediafile_id) {

  $form['mediafile_id'] = array('#type' => 'hidden', '#value' => $mediafile_id);

  // Get the mediafile.
  $mediafile = _csa_asset_mediafile_get($mediafile_id);

  // Not found (or other error)?
  if (!$mediafile) {
    drupal_goto();
  }

  $mediafile = reset($mediafile['items']['item']);

  return confirm_form($form,
                      t("Are you want to delete mediafile '@title'?", array('@title' => $mediafile['filename'])),
                      drupal_get_destination(),
                      t('This action cannot be undone.'),
                      t('Delete mediafile'),
                      t('Cancel')
  );
}

/**
 * Delete mediafile submit.
 */
function _csa_asset_mediafile_delete_form_submit($form, &$form_state) {

  $mediafile_id = $form_state['values']['mediafile_id'];

  // Get the mediafile.
  $mediafile = _csa_asset_mediafile_get($mediafile_id);

  // Not found (or other error)?
  if (!$mediafile) {
    drupal_goto();
  }

  $mediafile = reset($mediafile['items']['item']);

  switch ($form_state['values']['op']) {
    case t('Delete mediafile'):
      _csa_asset_mediafile_delete($mediafile_id, $mediafile['owner_id']);
      break;

    default:
    case t('Cancel'):
      drupal_set_message('You have canceled the deletion.');
      break;
  }

  // Will redirect back.
  drupal_goto();
}

/**
 *
 * @param $form_state
 * @param $mediafile_id
 */
function _csa_asset_mediafile_access_form(&$form_state, $mediafile_id) {

  $access = array();

  // Get the mediafile.
  $mediafile = _csa_asset_mediafile_get($mediafile_id);
  $asset = _csa_asset_get($mediafile['asset_id']);

  // Get the title.
  $title = _csa_asset_get_title($asset);

  // Set asset title.
  drupal_set_title($title);

  if (empty($form_state['storage'])) {
    // Get the mediafile.

    // Not found (or other error)?
    if (!$mediafile) {
      drupal_set_message(t('Enable to find mediafile.', 'error'));
      drupal_goto();
    }

    $mediafile = reset($mediafile['items']['item']);

    // Get ACL.
    $rights = _csa_asset_mediafile_get_acl($mediafile['mediafile_id'], $mediafile['owner_id']);

    foreach ($rights as $right) {
      if (!empty($right['aut_domain'])) {
        $form_state['storage']['access']['domains'][$right['aut_domain']] = $right['aut_domain'];
      }
      if (!empty($right['aut_realm'])) {
        $form_state['storage']['access']['realms'][$right['aut_realm']] = $right['aut_realm'];
      }
    }
  }

  // Get the basic form.
  $form = _csa_asset_mediafile_access_select_form($form_state, 0);

  $form['mediafile_id'] = array(
    '#type' => 'hidden',
    '#value' => $mediafile_id,
  );

  $form['submit']['start'] = array(
    '#type' => 'submit',
    '#value' => t(CSA_SUBMIT_TEXT_ASSET_MEDIAFILE_SUBMIT_ACCESS),
  );

  $form['submit']['cancel'] = array(
    '#type' => 'submit',
    '#value' => t(CSA_SUBMIT_TEXT_ASSET_MEDIAFILE_CANCEL_ACCESS),
  );

  $form['#validate'][] = '_csa_asset_mediafile_access_form_validate';
  $form['#submit'][] = '_csa_asset_mediafile_access_form_submit';
  return $form;
}

/**
 * Validate function for access form.
 */
function _csa_asset_mediafile_access_form_validate($form, $form_state) {
}

/**
 *
 * @param $form_state
 * @param $mediafile_id
 */
function _csa_asset_mediafile_access_form_submit($form, $form_state) {
  // Get the values;
  $values = $form_state['values'];

  switch ($values['op']) {
    case t(CSA_SUBMIT_TEXT_ASSET_MEDIAFILE_SUBMIT_ACCESS):

      $rights = array(
        'aut_domain' => empty($form_state['storage']['access']['domains']) ? array() : $form_state['storage']['access']['domains'],
        'aut_realm' => empty($form_state['storage']['access']['realms']) ? array() : $form_state['storage']['access']['realms'],
      );

      _csa_asset_mediafile_set_access($values['mediafile_id'], $rights);

      drupal_set_message(t('Access rules saved.'));
      drupal_goto();
      break;

    case t(CSA_SUBMIT_TEXT_ASSET_MEDIAFILE_CANCEL_ACCESS):
      drupal_goto();
  }
}

/**
 * Set the access on a mediafile.
 *
 * @param string $mediafile_id
 * @param array $options
 */
function _csa_asset_mediafile_set_access($mediafile_id, $options) {

  // Get the mediafile.
  $mediafile = _csa_asset_mediafile_get($mediafile_id);

  // Not found (or other error)?
  if (!$mediafile) {
    drupal_set_message(t('Enable to find mediafile.', 'error'));
    drupal_goto();
  }

  $mediafile = reset($mediafile['items']['item']);

  // Get ACL.
  $rights = _csa_asset_mediafile_get_acl($mediafile['mediafile_id'], $mediafile['owner_id']);

  $options['aut_app'] = array();

  foreach ($rights as $right) {
    if (!empty($right['aut_app'])) {
      $options['aut_app'][] = $right['aut_app'];
    }
  }

  _csa_asset_mediafile_set_acl($mediafile['mediafile_id'], $mediafile['owner_id'], $options);
}
