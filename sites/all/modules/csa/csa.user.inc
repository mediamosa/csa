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
 * All user related functions.
 */
define('CSA_SUBMIT_TEXT_PREFS_ADD_TRANSCODE', 'Add transcode >>');
define('CSA_SUBMIT_TEXT_PREFS_REMOVE_TRANSCODE', '<< Remove transcode');

define('CSA_SUBMIT_TEXT_PREFS_ADD_DEFAULT_METADATA', 'Add default metadata >>');
define('CSA_SUBMIT_TEXT_PREFS_REMOVE_DEFAULT_METADATA', '<< Remove default metadata');

define('CSA_SUBMIT_TEXT_PREFS_ADD_REQUIRED_METADATA', 'Add required metadata >>');
define('CSA_SUBMIT_TEXT_PREFS_REMOVE_REQUIRED_METADATA', '<< Remove required metadata');

define('CSA_SUBMIT_TEXT_PREFS_REMOVE_ACCESS_RULE', '<< Remove access rule');
define('CSA_SUBMIT_TEXT_PREFS_ADD_DOMAIN_REALM', 'Add domain / realm >>');
define('CSA_SUBMIT_TEXT_PREFS_ADD_APP', 'Add app >>');

define('CSA_SUBMIT_TEXT_PREFS_SAVE_MEDIAMANGEMENT', 'Save mediamangement settings');
define('CSA_SUBMIT_TEXT_PREFS_SAVE_CONNECTION', 'Save & test connection');

define('CSA_PREFIX_PREFS_MEDIAMANGEMENT', 'prefs_mm_');
define('CSA_PREFIX_PREFS_MEDIAMANGEMENT_GENERATE_STILL', CSA_PREFIX_PREFS_MEDIAMANGEMENT . 'generate_still');
define('CSA_PREFIX_PREFS_MEDIAMANGEMENT_DOWNLOADABLE', CSA_PREFIX_PREFS_MEDIAMANGEMENT . 'downloadable');
define('CSA_PREFIX_PREFS_MEDIAMANGEMENT_INHERITS_ACL', CSA_PREFIX_PREFS_MEDIAMANGEMENT . 'inherits_acl');
define('CSA_PREFIX_PREFS_MEDIAMANGEMENT_TAG', CSA_PREFIX_PREFS_MEDIAMANGEMENT . 'tag');

// Field in $user where user choose the transcodings.
define('CSA_USER_PREFS_UPLOAD_TRANSCODINGS', 'csa_transcodings');

// Default and required metadata fields.
define('CSA_USER_PREFS_DEFAULT_METADATA', 'csa_default_metadata');
define('CSA_USER_PREFS_REQUIRED_METADATA', 'csa_required_metadata');

// Default metadata upload.
define('CSA_USER_PREFS_DEFAULT_METADATA_UPLOAD', 'csa_default_metadata_upload');

/**
 * Used when user is provided in url.
 */
function _csa_user_prefs_form($form_state, $account) {

  global $user;

  if (empty($account)) {
    $account = user_load(intval($user->uid));
  }
  else {
    // Reload, for some reason the account contains old data.
    $account = user_load(intval($account->uid));
  }

  // We must use the MediaMosa login of the user.
  $user->csa_connection['app_name'] = _csa_get_user_value_connection($account, 'app_name');
  $user->csa_connection['shared_key'] = _csa_get_user_value_connection($account, 'shared_key');
  $user->csa_connection['mediamosa_url'] = _csa_get_user_value_connection($account, 'mediamosa_url');

  if (!_csa_mediamosa_status()) {
    drupal_set_message(t('The !mediamosa_connection does not work for !user.', array('!mediamosa_connection' => l('MediaMosa connection', 'user/' . $account->uid . '/csa_edit_connection'), '!user' => l($account->name, 'user/' . $account->uid))), 'error');
  }

  return _csa_prefs_form($form_state, $account);
}

/**
 * CSA edit users MediaMosa prefs.
 */
function _csa_prefs_form($form_state, $account = NULL) {
  global $user;

  if (empty($account)) {
    $account = user_load(intval($user->uid));
  }

  $form = array();

  // Build prefs form.
  $form = array_merge_recursive(
    _csa_prefs_access_form($form_state, $account),
    _csa_prefs_transcodings_form($form_state, $account),
    _csa_prefs_required_metadata_form($form_state, $account),
    _csa_prefs_default_metadata_form($form_state, $account),
    _csa_prefs_default_metadata_uploads_form($form_state, $account),
    _csa_prefs_mediamanagement_form($form_state, $account)
  );

  // Show we know.
  $form['uid'] = array(
    '#type' => 'hidden',
    '#value' => $account->uid,
  );

  $form['#submit'][] = '_csa_prefs_form_submit';
  return $form;
}

function _csa_prefs_form_submit($form, &$form_state) {
  $form_state['storage']['csa_prefs_op'] = empty($form_state['values']['op']) ? '' : $form_state['values']['op'];
}

/**
 * helper to know which section was last used.
 *
 * @param array $form_state
 */
function _csa_prefs_op_get($form_state) {
  return empty($form_state['storage']['csa_prefs_op']) ? (empty($_REQUEST['csa_prefs_op']) ? '' : $_REQUEST['csa_prefs_op']) : $form_state['storage']['csa_prefs_op'];
}

/**
 * Returns the access form of prefs form.
 */
function _csa_prefs_access_form($form_state, $account) {

  // CSA access rules.
  $csa_access_rules = $account->csa_access_rules;

  $select_access_rules = array();
  if (!empty($csa_access_rules['domains'])) {
    foreach ($csa_access_rules['domains'] as $domain) {
      $select_access_rules[t(CSA_ACCESS_TYPE_DOMAIN) . $domain] = t(CSA_ACCESS_TYPE_DOMAIN) . $domain;
    }
  }
  if (!empty($csa_access_rules['realms'])) {
    foreach ($csa_access_rules['realms'] as $realm) {
      $select_access_rules[t(CSA_ACCESS_TYPE_REALM) . $realm] = t(CSA_ACCESS_TYPE_REALM) . $realm;
    }
  }
  if (!empty($csa_access_rules['applications'])) {
    foreach ($csa_access_rules['applications'] as $id => $application) {
      $select_access_rules[$id] = t(CSA_ACCESS_TYPE_APP) . $application;
    }
  }

  $applications = _csa_get_applications($account);
  if (!empty($account->csa_access_rules['applications'])) {
    foreach ($account->csa_access_rules['applications'] as $selected_app => $name) {
      unset($applications[$selected_app]);
    }
  }

  $form['access'] = array(
    '#type' => 'fieldset',
    '#title' => t('Access rules'),
    '#collapsible' => TRUE,
    '#collapsed' => !in_array(_csa_prefs_op_get($form_state), array(t(CSA_SUBMIT_TEXT_PREFS_ADD_DOMAIN_REALM), t(CSA_SUBMIT_TEXT_PREFS_ADD_APP), t(CSA_SUBMIT_TEXT_PREFS_REMOVE_ACCESS_RULE))),
    '#description' => t("Access rules specify which users can see the uploaded media. You can enter 'domains', 'realms' and 'applications'. These access rules are used as default when publishing your media."),
  );

  $form['access']['access_input'] = array(
    '#prefix' => '<div class="csa-prefs-input">',
    '#suffix' => '</div>',
  );

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
    '#size' => 14,
    '#multiple' => TRUE,
    '#attributes' => array('class' => 'csa-select-prefs'),
  );
  $form['access']['access_output']['selected_access_rules_remove'] = array(
    '#type' => 'submit',
    '#value' => t(CSA_SUBMIT_TEXT_PREFS_REMOVE_ACCESS_RULE),// Fixed OP text, change op switch too.
  );

  $form['#validate'][] = '_csa_prefs_access_form_validate';
  $form['#submit'][] = '_csa_prefs_access_form_submit';

  return $form;
}

/**
 * Validate function for access form.
 */
function _csa_prefs_access_form_validate($form, $form_state) {
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
function _csa_prefs_access_form_submit($form, &$form_state) {
  // Get the values;
  $values = $form_state['values'];

  // Get the account we are changing.
  $account = user_load(intval($values['uid']));
  $save_account = FALSE;

  switch ($values['op']) {
    case t(CSA_SUBMIT_TEXT_PREFS_ADD_DOMAIN_REALM):
      // Get value.
      $value = $values['domain_realm'];

      // Is realm or domain?
      if (_csa_is_realm($value)) {
        $account->csa_access_rules['realms'][$value] = $value;

        // Show we added it.
        drupal_set_message(t('Access rule for realm !realm added.', array('!realm' => $value)));
        $save_account = TRUE;
      }
      elseif (_csa_is_domain($value)) {
        $account->csa_access_rules['domains'][$value] = $value;

        // Show we added it.
        drupal_set_message(t('Access rule for domain !domain added.', array('!domain' => $value)));
        $save_account = TRUE;
      }
      break;

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

    case t(CSA_SUBMIT_TEXT_PREFS_REMOVE_ACCESS_RULE):
      // Get the applications.
      $applications = _csa_get_applications($account);

      foreach ($values['selected_access_rules'] as $to_remove) {
        if (substr($to_remove, 0, drupal_strlen(t(CSA_ACCESS_TYPE_DOMAIN))) == t(CSA_ACCESS_TYPE_DOMAIN)) {
          $value = substr($to_remove, drupal_strlen(CSA_ACCESS_TYPE_DOMAIN));

          // Remove.
          unset($account->csa_access_rules['domains'][$value]);

          // Show we removed it.
          drupal_set_message(t('Access rule for domain !domain removed.', array('!domain' => $value)));
          $save_account = TRUE;
        }
        elseif (substr($to_remove, 0, drupal_strlen(t(CSA_ACCESS_TYPE_REALM))) == t(CSA_ACCESS_TYPE_REALM)) {
          $value = substr($to_remove, drupal_strlen(t(CSA_ACCESS_TYPE_REALM)));

          // Remove.
          unset($account->csa_access_rules['realms'][$value]);

          // Show we removed it.
          drupal_set_message(t('Access rule for realm !realm removed.', array('!realm' => $value)));
          $save_account = TRUE;
        }
        elseif (is_numeric($to_remove) && !empty($account->csa_access_rules['applications'][$to_remove])) {

          // Need the name in case application no longer exists.
          $name = (empty($applications[$to_remove]) ? $account->csa_access_rules['applications'][$to_remove] : $applications[$to_remove]);

          // Remove.
          unset($account->csa_access_rules['applications'][$to_remove]);

          // Show we removed it.
          drupal_set_message(t('Access rule for application !application removed.', array('!application' => $name)));
          $save_account = TRUE;
        }
      }
      break;
  }

  // Save it.
  if ($save_account) {
    user_save($account, array('csa_access_rules' => $account->csa_access_rules));
  }
}

/**
 * Precheck the access rules, if any are set. Will redirect when none are found.
 */
function _csa_prefs_access_precheck_rules() {
  global $user;

  if (empty($user->csa_access_rules['applications'])
/*      &&
      empty($user->csa_access_rules['domains']) &&
      empty($user->csa_access_rules['realms'])*/
    ) {
    drupal_set_message(t('Before you can publish, you have to setup access rules. Add at least one application to publish your media.'), 'error');
    drupal_goto('csa/prefs', array('csa_prefs_op' => CSA_SUBMIT_TEXT_PREFS_ADD_DOMAIN_REALM));
  }
}

function _csa_prefs_transcodings_form($form_state, $account) {

  // Get available transcodings.
  $transcodings = _csa_get_transcodings($account);

  // Unselect whats already selected.
  if (!empty($account->{CSA_USER_PREFS_UPLOAD_TRANSCODINGS})) {
    foreach ($account->{CSA_USER_PREFS_UPLOAD_TRANSCODINGS} as $id => $transcoding) {
      unset($transcodings[$id]);
    }
  }

  $form['transcodings'] = array(
    '#type' => 'fieldset',
    '#title' => t('Transcodings'),
    '#collapsible' => TRUE,
    '#collapsed' => !in_array(_csa_prefs_op_get($form_state), array(t(CSA_SUBMIT_TEXT_PREFS_ADD_TRANSCODE), t(CSA_SUBMIT_TEXT_PREFS_REMOVE_TRANSCODE))),
    '#description' => t("Transcodings can be automaticly made after an upload. Please specify the transcodings that are used after upload. Moving items to the right will make these transcoding used after upload."),
  );

  $form['transcodings']['transcoding_input'] = array(
    '#prefix' => '<div class="csa-prefs-input">',
    '#suffix' => '</div>',
  );

  $form['transcodings']['transcoding_input']['fieldset_1'] = array(
    '#type' => 'fieldset',
    '#title' => t('Available transcodings'),
    '#collapsible' => FALSE,
    '#collapsed' => FALSE,
  );
  $form['transcodings']['transcoding_input']['fieldset_1']['transcoding'] = array(
    '#type' => 'select',
    '#options' => $transcodings,
    '#size' => 8,
    '#multiple' => TRUE,
    '#attributes' => array('class' => 'csa-select-prefs'),
  );
  $form['transcodings']['transcoding_input']['fieldset_1']['transcoding_submit'] = array(
    '#type' => 'submit',
    '#value' => t(CSA_SUBMIT_TEXT_PREFS_ADD_TRANSCODE),
  );

  $form['transcodings']['transcoding_output'] = array(
    '#type' => 'fieldset',
    '#title' => t('Selected transcodings'),
    '#collapsible' => FALSE,
    '#collapsed' => FALSE,
    '#prefix' => '<div class="csa-prefs-output">',
    '#suffix' => '</div>',
  );
  $form['transcodings']['transcoding_output']['selected_transcodings'] = array(
    '#type' => 'select',
    '#options' => empty($account->{CSA_USER_PREFS_UPLOAD_TRANSCODINGS}) ? array() : $account->{CSA_USER_PREFS_UPLOAD_TRANSCODINGS},
    '#size' => 8,
    '#multiple' => TRUE,
    '#attributes' => array('class' => 'csa-select-prefs'),
  );
  $form['transcodings']['transcoding_output']['selected_transcodings_remove'] = array(
    '#type' => 'submit',
    '#value' => t(CSA_SUBMIT_TEXT_PREFS_REMOVE_TRANSCODE),
  );

  $form['#validate'][] = '_csa_prefs_transcodings_form_validate';
  $form['#submit'][] = '_csa_prefs_transcodings_form_submit';

  return $form;
}

/**
 * Validate function for transcodings form.
 */
function _csa_prefs_transcodings_form_validate($form, $form_state) {
  // Get the values;
  $values = $form_state['values'];

  switch ($values['op']) {
    case t(CSA_SUBMIT_TEXT_PREFS_ADD_TRANSCODE):
      break;

    case t(CSA_SUBMIT_TEXT_PREFS_REMOVE_TRANSCODE):
      break;
  }
}

/**
 * Submit function for transcodings form.
 */
function _csa_prefs_transcodings_form_submit($form, &$form_state) {

  global $user;

  // Get the values;
  $values = $form_state['values'];

  // Get the account we are changing.
  $account = user_load(intval($values['uid']));
  $save_account = FALSE;

  // We must use the MediaMosa login of the user.
  if ($account->uid != $user->uid) {
    $user->csa_connection['app_name'] = _csa_get_user_value_connection($account, 'app_name');
    $user->csa_connection['shared_key'] = _csa_get_user_value_connection($account, 'shared_key');
    $user->csa_connection['mediamosa_url'] = _csa_get_user_value_connection($account, 'mediamosa_url');
  }

  switch ($values['op']) {
    case t(CSA_SUBMIT_TEXT_PREFS_ADD_TRANSCODE):
      $csa_transcodings = _csa_user_prefs_upload_transcoding_get($account);
      $available_transcodings = _csa_get_transcodings($account);

      // Add it.
      if (!empty($values['transcoding'])) {
        foreach ($values['transcoding'] as $id) {
          if (isset($available_transcodings[$id])) {
            $csa_transcodings[$id] = $available_transcodings[$id];

            // Show we added it.
            drupal_set_message(t('The transcode !transcode has been added.', array('!transcode' => $available_transcodings[$id])));
            $save_account = TRUE;
          }
        }
      }
      break;

    case t(CSA_SUBMIT_TEXT_PREFS_REMOVE_TRANSCODE):
      $csa_transcodings = _csa_user_prefs_upload_transcoding_get($account);
      $available_transcodings = _csa_get_transcodings($account);

      if (!empty($values['selected_transcodings'])) {
        foreach ($values['selected_transcodings'] as $id) {
          unset($csa_transcodings[$id]);

          // Show we added it.
          drupal_set_message(t('The transcode !transcode has been removed.', array('!transcode' => $available_transcodings[$id])));
          $save_account = TRUE;
        }
      }
      break;
    }

  // Save it.
  if ($save_account) {
    user_save($account, array(CSA_USER_PREFS_UPLOAD_TRANSCODINGS => $csa_transcodings));
  }
}

/**
 * Form for default metadata preferences.
 */
function _csa_prefs_default_metadata_form($form_state, $account) {

  // Get available metadata.
  $metadata = _csa_asset_metadata_list_misc($account);

  // Unselect whats already selected.
  $csa_default_metadata = _csa_user_prefs_default_metadata_get($account);

  foreach ($csa_default_metadata as $prop_id => $name) {
    // Unset this metadata entry, so it can not be selected.
    foreach (array_keys($metadata['metadata_options']) as $metadata_group) {
      unset($metadata['metadata_options'][$metadata_group][$prop_id]);
    }
  }

  $form['metadata_default'] = array(
    '#type' => 'fieldset',
    '#title' => t('Default metadata selection'),
    '#collapsible' => TRUE,
    '#collapsed' => !in_array(_csa_prefs_op_get($form_state), array(t(CSA_SUBMIT_TEXT_PREFS_ADD_DEFAULT_METADATA), t(CSA_SUBMIT_TEXT_PREFS_REMOVE_DEFAULT_METADATA))),
    '#description' => t("You can specify which metadata fields are always used by default in your metadata form. If you specified fields that are required, then these are also considered 'default' and can't be removed as default metadata. Moving items to the right will make them default for metadata creation."),
  );

  $form['metadata_default']['fieldset_1'] = array(
    '#type' => 'fieldset',
    '#title' => t('Available metadata'),
    '#collapsible' => FALSE,
    '#collapsed' => FALSE,
    '#prefix' => '<div class="csa-prefs-input">',
    '#suffix' => '</div>',
  );
  $form['metadata_default']['fieldset_1']['metadata_default'] = array(
    '#type' => 'select',
    '#options' => $metadata['metadata_options'],
    '#size' => 8,
    '#multiple' => TRUE,
    '#attributes' => array('class' => 'csa-select-prefs'),
  );
  $form['metadata_default']['fieldset_1']['metadata_default_submit'] = array(
    '#type' => 'submit',
    '#value' => t(CSA_SUBMIT_TEXT_PREFS_ADD_DEFAULT_METADATA),
  );

  $form['metadata_default']['fieldset_2'] = array(
    '#type' => 'fieldset',
    '#title' => t('Selected metadata for default'),
    '#collapsible' => FALSE,
    '#collapsed' => FALSE,
    '#prefix' => '<div class="csa-prefs-output">',
    '#suffix' => '</div>',
  );
  $form['metadata_default']['fieldset_2']['selected_metadata_default'] = array(
    '#type' => 'select',
    '#options' => $csa_default_metadata,
    '#size' => 8,
    '#multiple' => TRUE,
    '#attributes' => array('class' => 'csa-select-prefs'),
  );
  $form['metadata_default']['fieldset_2']['selected_metadata_default_remove'] = array(
    '#type' => 'submit',
    '#value' => t(CSA_SUBMIT_TEXT_PREFS_REMOVE_DEFAULT_METADATA),
  );

  $form['#validate'][] = '_csa_prefs_default_metadata_form_validate';
  $form['#submit'][] = '_csa_prefs_default_metadata_form_submit';

  return $form;
}

/**
 * Validate function for default metadata form.
 */
function _csa_prefs_default_metadata_form_validate($form, $form_state) {
}

/**
 * Submit function for default metadata form.
 */
function _csa_prefs_default_metadata_form_submit($form, &$form_state) {

  global $user;

  // Get the values;
  $values = $form_state['values'];

  // Get the account we are changing.
  $account = user_load(intval($values['uid']));

  // We must use the MediaMosa login of the user.
  if ($account->uid != $user->uid) {
    $user->csa_connection['app_name'] = _csa_get_user_value_connection($account, 'app_name');
    $user->csa_connection['shared_key'] = _csa_get_user_value_connection($account, 'shared_key');
    $user->csa_connection['mediamosa_url'] = _csa_get_user_value_connection($account, 'mediamosa_url');
  }

  // Get available transcodings.
  $metadata = _csa_asset_metadata_list_misc();

  // So we know to save.
  $save_user = FALSE;

  switch ($values['op']) {
    case t(CSA_SUBMIT_TEXT_PREFS_ADD_DEFAULT_METADATA):
      $csa_default_metadata = _csa_user_prefs_default_metadata_get($account);

      // Add it.
      if (!empty($values['metadata_default'])) {
        foreach ($values['metadata_default'] as $prop_id) {
          if (!empty($metadata['metadata_propid_to_title'][$prop_id])) {
            $csa_default_metadata[$prop_id] = $metadata['metadata_propid_to_title'][$prop_id];
            // Show we added it.
            drupal_set_message(t("The default metadata '!metadata' has been added.", array('!metadata' => $metadata['metadata_propid_to_title'][$prop_id])));
            $save_user = TRUE;
          }
          else {
            drupal_set_message(t('Something went wrong here.'), 'error');
          }
        }
      }
      break;

    case t(CSA_SUBMIT_TEXT_PREFS_REMOVE_DEFAULT_METADATA):
      $csa_default_metadata = _csa_user_prefs_default_metadata_get($account);

      // global required.
      $csa_settings_required_metadata = _csa_settings_required_metadata_get();

      // User required.
      $csa_required_metadata = _csa_user_prefs_required_metadata_get($account);

      if (!empty($values['selected_metadata_default'])) {
        foreach ($values['selected_metadata_default'] as $prop_id) {
          if (isset($csa_settings_required_metadata[$prop_id])) {
            drupal_set_message(t("The required metadata '!metadata' can not be removed, this metadata is globally set as required by admin and is therefor automaticly default too.", array('!metadata' => $metadata['metadata_propid_to_title'][$prop_id])), 'error');
            continue;
          }

          if (isset($csa_required_metadata[$prop_id])) {
            drupal_set_message(t("The required metadata '!metadata' can not be removed, this metadata is set as required by you and is therefor automaticly default too, see section 'Required metadata selection'.", array('!metadata' => $metadata['metadata_propid_to_title'][$prop_id])), 'error');
            continue;
          }

          unset($csa_default_metadata[$prop_id]);

          // Show we added it.
          drupal_set_message(t("The default metadata '!metadata' has been removed.", array('!metadata' => $metadata['metadata_propid_to_title'][$prop_id])));
          $save_user = TRUE;
        }
      }
      break;
  }

  // Save it.
  if ($save_user) {
    user_save($account, array(CSA_USER_PREFS_DEFAULT_METADATA => $csa_default_metadata));
  }
}

/**
 * Form for required metadata preferences.
 */
function _csa_prefs_required_metadata_form($form_state, $account) {

  // Get available transcodings.
  $metadata = _csa_asset_metadata_list_misc($account);

  // Unselect whats already selected.
  $csa_required_metadata = _csa_user_prefs_required_metadata_get($account);
  foreach ($csa_required_metadata as $prop_id => $name) {
    // Unset this metadata entry, so it can not be selected.
    foreach (array_keys($metadata['metadata_options']) as $metadata_group) {
      unset($metadata['metadata_options'][$metadata_group][$prop_id]);
    }
  }

  $form['metadata_required'] = array(
    '#type' => 'fieldset',
    '#title' => t('Required metadata selection'),
    '#collapsible' => TRUE,
    '#collapsed' => !in_array(_csa_prefs_op_get($form_state), array(t(CSA_SUBMIT_TEXT_PREFS_ADD_REQUIRED_METADATA), t(CSA_SUBMIT_TEXT_PREFS_REMOVE_REQUIRED_METADATA))),
    '#description' => t("This selection of metadata fields will always be required to be entered and you'll not be able to publish assets that are missing these metadata fields. Moving items to the right will make them required for metadata creation."),
  );

  $form['metadata_required']['fieldset_1'] = array(
    '#type' => 'fieldset',
    '#title' => t('Available metadata'),
    '#collapsible' => FALSE,
    '#collapsed' => FALSE,
    '#prefix' => '<div class="csa-prefs-input">',
    '#suffix' => '</div>',
  );
  $form['metadata_required']['fieldset_1']['metadata_required'] = array(
    '#type' => 'select',
    '#options' => $metadata['metadata_options'],
    '#size' => 8,
    '#multiple' => TRUE,
    '#attributes' => array('class' => 'csa-select-prefs'),
  );
  $form['metadata_required']['fieldset_1']['metadata_required_submit'] = array(
    '#type' => 'submit',
    '#value' => t(CSA_SUBMIT_TEXT_PREFS_ADD_REQUIRED_METADATA),
  );

  $form['metadata_required']['fieldset_2'] = array(
    '#type' => 'fieldset',
    '#title' => t('Selected metadata for required'),
    '#collapsible' => FALSE,
    '#collapsed' => FALSE,
    '#prefix' => '<div class="csa-prefs-output">',
    '#suffix' => '</div>',
  );
  $form['metadata_required']['fieldset_2']['selected_metadata_required'] = array(
    '#type' => 'select',
    '#options' => $csa_required_metadata,
    '#size' => 8,
    '#multiple' => TRUE,
    '#attributes' => array('class' => 'csa-select-prefs'),
  );
  $form['metadata_required']['fieldset_2']['selected_metadata_required_remove'] = array(
    '#type' => 'submit',
    '#value' => t(CSA_SUBMIT_TEXT_PREFS_REMOVE_REQUIRED_METADATA),
  );

  $form['#validate'][] = '_csa_prefs_required_metadata_form_validate';
  $form['#submit'][] = '_csa_prefs_required_metadata_form_submit';

  return $form;
}

/**
 * Validate function for default metadata form.
 */
function _csa_prefs_required_metadata_form_validate($form, $form_state) {
}

/**
 * Submit function for default metadata form.
 */
function _csa_prefs_required_metadata_form_submit($form, &$form_state) {

  global $user;

  // Get the values;
  $values = $form_state['values'];

  // Get the account we are changing.
  $account = user_load(intval($values['uid']));

  // We must use the MediaMosa login of the user.
  if ($account->uid != $user->uid) {
    $user->csa_connection['app_name'] = _csa_get_user_value_connection($account, 'app_name');
    $user->csa_connection['shared_key'] = _csa_get_user_value_connection($account, 'shared_key');
    $user->csa_connection['mediamosa_url'] = _csa_get_user_value_connection($account, 'mediamosa_url');
  }

  // Get available transcodings.
  $metadata = _csa_asset_metadata_list_misc($account);

  // So we know to save.
  $save_user = FALSE;

  switch ($values['op']) {
    case t(CSA_SUBMIT_TEXT_PREFS_ADD_REQUIRED_METADATA):
      $csa_required_metadata = _csa_user_prefs_required_metadata_get($account);

      // Add it.
      if (!empty($values['metadata_required'])) {
        foreach ($values['metadata_required'] as $prop_id) {
          if (isset($metadata['metadata_propid_to_title'][$prop_id])) {
            $csa_required_metadata[$prop_id] = $metadata['metadata_propid_to_title'][$prop_id];

            // Show we added it.
            drupal_set_message(t("The required metadata '!metadata' has been added.", array('!metadata' => $metadata['metadata_propid_to_title'][$prop_id])));
            $save_user = TRUE;
          }
        }
      }
      break;

    case t(CSA_SUBMIT_TEXT_PREFS_REMOVE_REQUIRED_METADATA):
      $csa_required_metadata = _csa_user_prefs_required_metadata_get($account);

      // global required.
      $csa_settings_required_metadata = _csa_settings_required_metadata_get();

      if (!empty($values['selected_metadata_required'])) {
        foreach ($values['selected_metadata_required'] as $prop_id) {
          if (isset($csa_settings_required_metadata[$prop_id])) {
            drupal_set_message(t("The required metadata '!metadata' can not be removed, this metadata is globally set as required by admin.", array('!metadata' => $metadata['metadata_propid_to_title'][$prop_id])), 'error');
            continue;
          }

          unset($csa_required_metadata[$prop_id]);

          // Show we added it.
          drupal_set_message(t("The required metadata '!metadata' has been removed.", array('!metadata' => $metadata['metadata_propid_to_title'][$prop_id])));
          $save_user = TRUE;
        }
      }
      break;
  }

  // Save it.
  if ($save_user) {
    user_save($account, array(CSA_USER_PREFS_REQUIRED_METADATA => $csa_required_metadata));
  }
}

/**
 * Mediamanagement prefs form.
 */
function _csa_prefs_mediamanagement_form($form_state, $account) {
   $form['mediamanagement'] = array(
    '#type' => 'fieldset',
    '#title' => t('Media management'),
    '#collapsible' => TRUE,
    '#collapsed' => !in_array(_csa_prefs_op_get($form_state), array(t(CSA_SUBMIT_TEXT_PREFS_SAVE_MEDIAMANGEMENT))),
    '#description' => t("You can specify settings here for your media."),
  );
  $form['mediamanagement'][CSA_PREFIX_PREFS_MEDIAMANGEMENT_GENERATE_STILL] = array(
    '#type' => 'checkbox',
    '#title' => t('Generate a still for each upload'),
    '#default_value' => _csa_get_user_value_prefs($account, CSA_PREFIX_PREFS_MEDIAMANGEMENT_GENERATE_STILL),
  );
  $form['mediamanagement'][CSA_PREFIX_PREFS_MEDIAMANGEMENT_DOWNLOADABLE] = array(
    '#type' => 'checkbox',
    '#title' => t('Make all uploads downloadable'),
    '#default_value' => _csa_get_user_value_prefs($account, CSA_PREFIX_PREFS_MEDIAMANGEMENT_DOWNLOADABLE)
  );
  $form['mediamanagement'][CSA_PREFIX_PREFS_MEDIAMANGEMENT_INHERITS_ACL] = array(
    '#type' => 'checkbox',
    '#title' => t('Transcodes inherits ACL'),
    '#default_value' => _csa_get_user_value_prefs($account, CSA_PREFIX_PREFS_MEDIAMANGEMENT_INHERITS_ACL)
  );
  $form['mediamanagement'][CSA_PREFIX_PREFS_MEDIAMANGEMENT_TAG] = array(
    '#type' => 'textfield',
    '#title' => t('Default tag for new uploads'),
    '#default_value' => _csa_get_user_value_prefs($account, CSA_PREFIX_PREFS_MEDIAMANGEMENT_TAG),
    '#maxlength' => 32, // tags max 32 chars.
  );
  $form['mediamanagement']['submit'] = array(
    '#type' => 'submit',
    '#value' => t(CSA_SUBMIT_TEXT_PREFS_SAVE_MEDIAMANGEMENT),
  );

  $form['#validate'][] = '_csa_prefs_mediamanagement_form_validate';
  $form['#submit'][] = '_csa_prefs_mediamanagement_form_submit';

  return $form;
}

/**
 * Validate function for mediamanagement form.
 */
function _csa_prefs_mediamanagement_form_validate($form, $form_state) {
}

/**
 * Submit function for mediamanagement form.
 */
function _csa_prefs_mediamanagement_form_submit($form, &$form_state) {
  $values = $form_state['values'];

  switch ($values['op']) {
    case t(CSA_SUBMIT_TEXT_PREFS_SAVE_MEDIAMANGEMENT):
      // Get the account we are changing.
      $account = user_load(intval($values['uid']));

      // See if stuff has changed.
      $csa_prefs = empty($account->csa_prefs) ? array() : $account->csa_prefs;

      foreach ($values as $name => $value) {
        if (substr($name, 0, drupal_strlen(CSA_PREFIX_PREFS_MEDIAMANGEMENT)) == CSA_PREFIX_PREFS_MEDIAMANGEMENT) {
          $csa_prefs[$name] = $value;
        }
      }

      // Save it.
      user_save($account, array('csa_prefs' => $csa_prefs));

      // Show we saved it.
      drupal_set_message(t('The mediamanagement settings are saved.'));
      break;
  }
}

/**
 * Mediamanagement metadata upload form.
 */
function _csa_prefs_default_metadata_uploads_form($form_state, $account) {

  // Get default metadata of user.
  $default_values_metadata = _csa_user_prefs_default_metadata_upload_get($account);

  $form['metadata_default_uploads'] = array(
    '#type' => 'fieldset',
    '#title' => t('Default metadata for uploads'),
    '#collapsible' => TRUE,
    '#collapsed' => !in_array(_csa_prefs_op_get($form_state), array(t(CSA_ASSET_METADATA_SAVE_METADATA), t(CSA_ASSET_METADATA_ADD_FIELD_OPTION))),
    '#description' => t('The metadata you enter here will be applied as default values for all uploads. Empty fields will be ignored. Use =filename if you want to the filename in this field, f.e. title can automaticly the filename after the upload.'),
  );

  // Get the metadata form.
  $form['metadata_default_uploads']['form'] = _csa_asset_metadata_form($form_state, NULL, -1, CSA_MODE_QUERY, array('button_save_metadata_and_done' => FALSE, 'button_cancel' => FALSE, 'skip_required_fields' => TRUE), $default_values_metadata);

  $form['#validate'][] = '_csa_prefs_default_metadata_uploads_form_validate';
  $form['#submit'][] = '_csa_prefs_default_metadata_uploads_form_submit';

  return $form;
}

/**
 * Validate function for metadata upload form.
 */
function _csa_prefs_default_metadata_uploads_form_validate($form, &$form_state) {
  _csa_asset_metadata_form_validate($form, $form_state);
}


/**
 * Validate input.
 *
 * @param array $form
 * @param array $form_state
 */
function _csa_prefs_default_metadata_uploads_form_submit($form, &$form_state) {
  $values = $form_state['values'];

  switch ($values['op']) {
    case t(CSA_ASSET_METADATA_ADD_FIELD_OPTION):
      $form_state['storage']['metadata_group_add'] = array_merge(isset($form_state['storage']['metadata_group_add']) ? $form_state['storage']['metadata_group_add'] : array(), $values['metadata_group_add']);

      // Prevent losing our posted data.
      $form_state['rebuild'] = TRUE;
      break;

    case t(CSA_ASSET_METADATA_SAVE_METADATA):
      $metadata = array();

      // Get the account we are changing.
      $account = user_load(intval($values['uid']));

      foreach ($values as $name => $value) {
        // Any empty values we skip.
        if (!is_string($value) || trim($value) == '') {
          continue;
        }

        // Name is in format of [CSA_ASSET_METADATA_PREFIX] [name] [_#]
        if (substr($name, 0, drupal_strlen(CSA_ASSET_METADATA_PREFIX)) ==  CSA_ASSET_METADATA_PREFIX) {
          $name = substr($name, drupal_strlen(CSA_ASSET_METADATA_PREFIX));

          $name = explode('_', $name);
          $key = array_pop($name);// remove key.

          $metadata[implode('_', $name)][] = trim($value);
        }
      }

      // Save it.
      user_save($account, array(CSA_USER_PREFS_DEFAULT_METADATA_UPLOAD => $metadata));
      drupal_set_message(t('Default metadata for uploads saved.'));
      break;
  }
}

/**
 * CSA edit Mediamosa connection.
 */
function _csa_edit_connection($form_state, $account = NULL) {
  $form = array();

  global $user;

  if (empty($account)) {
    $account = $user;
  }

  // Get default/current values.
  $mediamosa_url = _csa_get_user_value_connection($account, 'mediamosa_url');
  $app_name = _csa_get_user_value_connection($account, 'app_name');
  $shared_key = _csa_get_user_value_connection($account, 'shared_key');
  $mediamosa_user = _csa_get_user_value_connection($account, 'mediamosa_user', 'csa.admin');

  $form['user'] = array(
    '#type' => 'fieldset',
    '#title' => t('User settings'),
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
  );

  $form['user']['mediamosa_url'] = array(
    '#type' => 'textfield',
    '#title' => t('MediaMosa URL for users'),
    '#description' => t('Enter the URL of the MediaMosa interface you want to connect to.'),
    '#default_value' => empty($mediamosa_url) ? variable_get('mediamosa_connector_url', NULL) : $mediamosa_url,
    '#maxlength' => 255,
    '#required' => TRUE,
  );
  $form['user']['app_name'] = array(
    '#type' => 'textfield',
    '#title' => t('MediaMosa application'),
    '#description' => t('The MediaMosa application is the name of the client application in MediaMosa.'),
    '#default_value' => $app_name,
    '#maxlength' => 255,
    '#required' => TRUE,
  );
  $form['user']['shared_key'] = array(
    '#type' => 'textfield',
    '#title' => t('MediaMosa shared key'),
    '#description' => t("The shared key for the client application is the specified password the application."),
    '#default_value' => $shared_key,
    '#maxlength' => 255,
    '#required' => TRUE,
  );
  $form['user']['mediamosa_user'] = array(
    '#type' => 'textfield',
    '#title' => t('MediaMosa owner'),
    '#description' => t("The owner for newly uploaded files for this connection."),
    '#default_value' => $mediamosa_user,
    '#maxlength' => 255,
    '#required' => TRUE,
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t(CSA_SUBMIT_TEXT_PREFS_SAVE_CONNECTION),
  );

  $form['user']['uid'] = array(
    '#type' => 'value',
    '#value' => $account->uid,
  );


  return $form;
}

/**
 * Validate the mediamosa connection form.
 */
function _csa_edit_connection_validate($form, &$form_state) {
  $values = $form_state['values'];

  // if app name is not given, then the other 2 must be empty.
  if (empty($values['app_name'])) {
    if (!empty($values['shared_key']) || !empty($values['mediamosa_url'])) {
      form_set_error('app_name', t('MediaMosa application is required.'));
    }
  }
}

/**
 * Submit handler.
 */
function _csa_edit_connection_submit($form, &$form_state) {
  $values = $form_state['values'];

  switch ($values['op']) {
    case t(CSA_SUBMIT_TEXT_PREFS_SAVE_CONNECTION):

      // Setup connector.
      $mediamosa_connector = new mediamosa_connector($values['app_name'], $values['shared_key'], $values['mediamosa_url']);

      // Do reconnect.
      $mediamosa_connector->reset_session();

      // Get version.
      $result = $mediamosa_connector->request('version'); // version should always work.

      if ($result) {
        drupal_set_message(t('Connection was made, login ok.'));
      }
      else {
        drupal_set_message(t('Login failed, check settings.'), 'error');
      }

      // Get the account we are changing.
      $account = user_load(intval($values['uid']));

      foreach (array('submit', 'uid', 'op', 'form_build_id', 'form_token', 'form_id') as $name) {
        unset($values[$name]);
      }

      // Store in the users data.
      user_save($account, array('csa_connection' => $values));

      // Show we saved it.
      drupal_set_message(t('The connection settings are saved.'));
      break;
  }
}

/**
 * Get the user required metadata setting.
 */
function _csa_user_prefs_required_metadata_get($account = NULL) {
  global $user;

  if (!isset($account)) {
    $account = $user;
  }

  $csa_required_metadata = _csa_settings_required_metadata_get() + (empty($account->{CSA_USER_PREFS_REQUIRED_METADATA}) ? array() : $account->{CSA_USER_PREFS_REQUIRED_METADATA});
  foreach ($csa_required_metadata as $key => $value) {
    if (empty($value)) {
      unset($csa_required_metadata[$key]);
    }
  }

  ksort($csa_required_metadata, SORT_NUMERIC);
  return $csa_required_metadata;
}

/**
 * Get the user default metadata setting.
 */
function _csa_user_prefs_default_metadata_get($account = NULL) {
  global $user;

  if (!isset($account)) {
    $account = $user;
  }

  $csa_default_metadata = _csa_user_prefs_required_metadata_get() + (empty($account->{CSA_USER_PREFS_DEFAULT_METADATA}) ? array() : $account->{CSA_USER_PREFS_DEFAULT_METADATA});
  foreach ($csa_default_metadata as $key => $value) {
    if (empty($value)) {
      unset($csa_default_metadata[$key]);
    }
  }

  ksort($csa_default_metadata, SORT_NUMERIC);
  return $csa_default_metadata;
}

/**
 * Get the user default transcode profiles when upload was completed.
 */
function _csa_user_prefs_upload_transcoding_get($account = NULL) {
  global $user;

  if (!isset($account)) {
    $account = $user;
  }

  return empty($user->{CSA_USER_PREFS_UPLOAD_TRANSCODINGS}) ? array() : $user->{CSA_USER_PREFS_UPLOAD_TRANSCODINGS};
}

/**
 * Get the user default metadata for uploads.
 */
function _csa_user_prefs_default_metadata_upload_get($account = NULL) {
  global $user;

  if (!isset($account)) {
    $account = $user;
  }

  return empty($user->{CSA_USER_PREFS_DEFAULT_METADATA_UPLOAD}) ? array() : $user->{CSA_USER_PREFS_DEFAULT_METADATA_UPLOAD};
}
