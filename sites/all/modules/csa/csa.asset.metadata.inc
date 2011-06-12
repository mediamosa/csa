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
 * Asset metadata functions.
 */

define('CSA_ASSET_METADATA_SAVE_METADATA', 'Save');
define('CSA_ASSET_METADATA_SAVE_METADATA_AND_NEXT', 'Save & next asset');
define('CSA_ASSET_METADATA_CANCEL', 'Cancel');
define('CSA_ASSET_METADATA_ADD_FIELD_OPTION', 'Add field option');

define('CSA_ASSET_METADATA_PREFIX', 'asset_metadata_');

/**
 * Retrieve the metadata def.
 */
function _csa_asset_metadata_list($options = array()) {

  // Cache.
  static $metadata_tags = array();

  if (!empty($metadata_tags)) {
    return $metadata_tags;
  }

  // Get the metadata tags.
  try {
    $offset = 0;

    while (true) {
      $response = mediamosa_connector::response2array(_csa_mediamosa_request('metadata_tag', array('data' => array('order_by' => 'prop_name', 'limit' => mediamosa_connector::MAX_RESULT_SET, 'offset' => $offset), 'fatal' => true)));

      if (!$response) {
        return array();
      }

      // Get the info.
      foreach ($response['items']['item'] as $item) {
        $metadata_tags[$item['prop_group']][$item['name']] = array(
          'name' => $item['name'],
          'prop_group' => $item['prop_group'],
          'prop_id' => $item['prop_id'],
          'type' => $item['type'],
        );
      }

      // more?
      if ($response['header']['item_count_total'] != mediamosa_connector::MAX_RESULT_SET) {
        break;
      }

      // add for next.
      $offset += $response['header']['item_count_total'];
    }
  }
  catch (Exception $e) {
    return array();
  }

  return $metadata_tags;
}

/**
 * Extended version of _csa_asset_metadata_list;
 */
function _csa_asset_metadata_list_misc() {
  static $metadata = array();

  if (!empty($metadata)) {
    return $metadata;
  }

  $asset_metadata = _csa_asset_metadata_list();

  // Metadata groups, like czp, dc.
  $metadata_propid_to_type = $metadata_propid_to_title = $metadata_options = $metadata_groups = array();

  foreach (array_keys($asset_metadata) as $metadata_group) {
    $metadata_groups[$metadata_group] = array('title' => t(_csa_name2title($metadata_group)));
  }

  // For drop downlists (selects).
  foreach ($metadata_groups as $metadata_group => $metadata_group_details) {
    foreach ($asset_metadata[$metadata_group] as $metadata) {
      $name = $metadata['name'];
      $title = t(_csa_name2title($name));
      $metadata_options[$metadata_group_details['title']][$asset_metadata[$metadata_group][$name]['prop_id']] = $title;
      $metadata_propid_to_title[$asset_metadata[$metadata_group][$name]['prop_id']] = $title;
      $metadata_propid_to_name[$asset_metadata[$metadata_group][$name]['prop_id']] = $name;
      $metadata_propid_to_type[$asset_metadata[$metadata_group][$name]['prop_id']] = $metadata['type'];
    }
  }

  $metadata = array('metadata' => $asset_metadata, 'metadata_groups' => $metadata_groups, 'metadata_options' => $metadata_options, 'metadata_propid_to_title' => $metadata_propid_to_title, 'metadata_propid_to_name' => $metadata_propid_to_name, 'metadata_propid_to_type' => $metadata_propid_to_type);
  return $metadata;
}

/**
 * The asset form metadata part.
 *
 * Warning used in 3 places;
 *
 * - On selection of assets (batch).
 * - On single asset edit.
 * - On prefs default metadata of user.
 *
 *
 * @param array $form_state
 * @param string $asset_id
 *   The asset ID (optional)
 * @param array $options
 *   Possible options for form.
 * @param array $default_values_metadata
 *   Values to use in form, will be used to build up form. Only when asset_id
 *   is NULL.
 */
function _csa_asset_metadata_form(&$form_state, $asset_id = NULL, $offset = -1, $mode = CSA_MODE_QUERY, $options = array(), $default_values_metadata = array()) {

  // Make sure these combos are met.
  assert((!is_null($asset_id) && empty($default_values_metadata)) || is_null($asset_id));

  // Setup defaults.
  $options += array(
    'type' => empty($_POST['type']) ? '' : $_POST['type'],
    'button_cancel' => empty($_POST['button_cancel']) ? TRUE : $_POST['button_cancel'],
    'skip_required_fields' => empty($_POST['skip_required_fields']) ? FALSE : $_POST['skip_required_fields'],
  );

  // CSS.
  drupal_add_css(drupal_get_path('module', 'csa') . '/csa.css', 'module');

  // Empty by default.
  $asset = array();

  // Get the asset.
  if (!empty($asset_id)) {
    $asset = _csa_asset_get($asset_id);

    if (empty($asset) || empty($asset['items']['item'])) {
      return array('foo' => array('#value' => t('Unable to find asset.')));
    }

    // Clean asset.
    $asset = reset($asset['items']['item']);
  }

  // Metadata.
  $metadata = _csa_asset_metadata_list_misc();

  // Get the groups.
  $metadata_groups = array_keys($metadata['metadata_groups']);

  // Default (has also required combined).
  $default_metadata = _csa_user_prefs_default_metadata_get();

  // Form metadata.
  $form_metadata = array();

  // Selected extra items.
  $to_add = empty($form_state['storage']['metadata_group_add']) ? array() : $form_state['storage']['metadata_group_add'];

  // When no asset is supplied, we haven't done anything yet, and we have default settings for metadata,
  // the set the default selection.
  if (empty($asset_id) && empty($form_state['storage']) && empty($to_add) && !empty($default_metadata)) {
    $to_add = array_keys($default_metadata);
    $form_state['storage']['metadata_group_add'] = $to_add;

      // Store asset_id.
    $form['to_add'] = array(
      '#type' => 'hidden',
      '#value' => serialize($to_add),
    );
  }

  $prop_ids_keys = array();

  // Now build fields in use.
  foreach ($metadata_groups as $metadata_group) {
    foreach ($metadata['metadata_propid_to_name'] as $prop_id => $name) {

      // Will use $asset despite of $default_values_metadata.
      if (empty($asset) && !empty($default_values_metadata[$name])) {
        if (is_array($default_values_metadata[$name])) {
          $metadata_values = $default_values_metadata[$name];
          $default_values_metadata[$name] = TRUE;

          $found = 0;
          $key = array_search($prop_id, $to_add);
          if ($key !== FALSE) {
            $metadata_values[] = '';
            unset($to_add[$key]);
          }
        }
        else {
          continue;
        }
      }
      elseif (!empty($asset) && isset($metadata['metadata'][$metadata_group][$name])) {
        $prop_id = $metadata['metadata'][$metadata_group][$name]['prop_id'];

        // If not in asset as value and not default, then skip it.
        if ((!isset($asset[$metadata_group][$name]) || $asset[$metadata_group][$name] == '') && !isset($default_metadata[$prop_id])) {
          continue;
        }

        // Values.
        $metadata_values = isset($asset[$metadata_group][$name]) ? (is_array($asset[$metadata_group][$name]) ? $asset[$metadata_group][$name] : array($asset[$metadata_group][$name])) : array('');
      }
      elseif (in_array($prop_id, $to_add)) {
        $found = 0;
        $metadata_values = array();
        foreach ($to_add as $key => $prop_id_to_add) {
          if ($prop_id_to_add == $prop_id) {
            $metadata_values[] = '';
            unset($to_add[$key]);
          }
        }
      }
      else {
        continue;
      }

      foreach ($metadata_values as $weight => $value) {
        $prop_ids_keys[$prop_id] = empty($prop_ids_keys[$prop_id]) ? 1 : $prop_ids_keys[$prop_id] + 1;

        $type = $metadata['metadata_propid_to_name'][$prop_id];

        $options['weight'] = $weight + (isset($default_metadata[$prop_id]) ? 0 : 50);

        // Add to form.
        $form_metadata += _csa_asset_metadata_form_add_item($form_state, $value, $type, $prop_ids_keys[$prop_id], $name, $prop_id, $options);
      }
    }
  }

  // Unset options for metadata that are double and are not allowed to be double.
  $metadata_options = $metadata['metadata_options'];

  // Whats allowed as multiple?
  $allowed_multiple = _csa_settings_multiple_metadata_get();

  foreach ($metadata_options as $group_name => $metadata_group) {
    foreach ($metadata_group as $prop_id => $name) {
      if (isset($allowed_multiple[$prop_id]) || (!isset($allowed_multiple[$prop_id]) && !isset($prop_ids_keys[$prop_id]))) {
        continue;
      }
      // Not allowed as multiple.
      unset($metadata_options[$group_name][$prop_id]);
    }
  }

  // Add fieldset for group.
  $form['metadata_select'] = array(
    '#type' => 'fieldset',
    '#title' => 'Add metadata field option',
    '#weight' => -5,
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
    '#prefix' => '<div class="csa-metadata-field-add">',
    '#suffix' => '</div>',
  );

  $form['metadata_select']['metadata_group_add'] = array(
    '#type' => 'select',
    '#options' => $metadata_options,
    '#size' => 8,
    '#multiple' => TRUE,
    '#attributes' => array('class' => 'csa-metadata-field-add'),
  );
  // Submit buttons
  $form['metadata_select']['submit'] = array(
    '#type' => 'submit',
    '#value' => t(CSA_ASSET_METADATA_ADD_FIELD_OPTION),
  );

  // Descriptions.
  $descriptions = array(
    'title' => t('Title of the asset.'),
  );

  // Store asset_id.
  $form['asset_id'] = array(
    '#type' => 'hidden',
    '#value' => $asset_id,
  );

  // Store offset.
  $form['offset'] = array(
    '#type' => 'hidden',
    '#value' => $offset,
  );

  // Store mode.
  $form['mode'] = array(
    '#type' => 'hidden',
    '#value' => $mode,
  );

  if (!empty($form_metadata)) {
    // Add fieldset for metadata edit.
    $form['metadata'] = array(
      '#type' => 'fieldset',
      '#title' => 'Asset metadata',
      '#weight' => 0,
      '#collapsible' => TRUE,
      '#collapsed' => FALSE
    );

    // Add the form.
    $form['metadata']['form'] = $form_metadata;

    $form['metadata']['buttons']['save'] = array(
      '#type' => 'submit',
      '#value' => t(CSA_ASSET_METADATA_SAVE_METADATA),
    );

    // Submit buttons
    if (!empty($GLOBALS['asset_next_url'])) {
      $form['metadata']['asset_next_url'] = array(
        '#type' => 'hidden',
        '#value' => $GLOBALS['asset_next_url'],
      );

      $form['metadata']['buttons']['save_next'] = array(
        '#type' => 'submit',
        '#value' => t(CSA_ASSET_METADATA_SAVE_METADATA_AND_NEXT),
      );
    }

    if (!empty($options['button_cancel'])) {
      $form['metadata']['buttons']['cancel'] = array(
        '#type' => 'submit',
        '#value' => t(CSA_ASSET_METADATA_CANCEL),
      );
    }
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
 * Helper for adding form item.
 */
function _csa_asset_metadata_form_add_item($form_state, $value, $type, $key, $name, $prop_id, $options = array()) {

  $options += array(
    'skip_required_fields' => FALSE,
    'weight' => 0,
  );

  $added = isset($GLOBALS['added']) ? $GLOBALS['added'] : array();

  // Types, not defined = textfield.
  $types = array('textarea' => array('description', 'beschrijving'));

  // Higher weights for these;
  $more_important = array('title', 'titel');

  // Required.
  $required_metadata = $options['skip_required_fields'] ? array() :  _csa_user_prefs_required_metadata_get();

  // Fix the name so we can find them.
  $metadata_name = CSA_ASSET_METADATA_PREFIX . $name . '_' . $key;

  // Descriptions.
  $descriptions = array(
    'title' => t('Title of the mediafile'),
    'titel' => t('Title of the mediafile'),
  );

  // Value, if any.
  $value = isset($form_state['values'][$metadata_name]) ? $form_state['values'][$metadata_name] : $value;

  // Set type.
  $type = (in_array($name, $types['textarea']) ? 'textarea' : NULL);
  $type = (isset($type) ? $type : (drupal_strlen((string) $value) > 100 ? 'textarea' : 'textfield'));
  $is_important = in_array($name, $more_important);
  $is_required = isset($required_metadata[$prop_id]) && !isset($added[$name]) ? TRUE : FALSE;

  $form_metadata = array(
    '#type' => $type,
    '#title' => t(_csa_name2title($name)),
    '#default_value' => $value,
    '#required' => $is_required,
    '#weight' => ($is_required ? ($is_important ? -1500 : -1000) : ($is_important ? -500 : 0)) + $options['weight'],
  );

  // Set description.
  if (isset($descriptions[$name])) {
    $form_metadata['#description'] = $descriptions[$name];
  }

  // To prevent items that are double and double required.
  $GLOBALS['added'][$name] = $name;

  return array($metadata_name => $form_metadata);
}

/**
 * Validate function for access form.
 *
 * @see _csa_box_metadata_form_validate()
 */
function _csa_asset_metadata_form_validate($form, &$form_state) {

  $values = $form_state['values'];

  switch ($values['op']) {
    case CSA_ASSET_METADATA_ADD_FIELD_OPTION:
      // When we press add field option, prevent missing field problems.
      form_set_error(NULL, '', TRUE);
      drupal_get_messages('error', TRUE);

      if (empty($values['metadata_group_add'])) {
        form_set_error('metadata_group_add', t('Select one or more types to add to selection of metadata.'), TRUE);
      }
      break;

    case t(CSA_ASSET_METADATA_SAVE_METADATA_AND_NEXT):
    case t(CSA_ASSET_METADATA_SAVE_METADATA):
      $metadata = array();
      foreach ($values as $name => $value) {
        // Name is in format of [CSA_ASSET_METADATA_PREFIX] [name] [_#]
        if (substr($name, 0, drupal_strlen(CSA_ASSET_METADATA_PREFIX)) ==  CSA_ASSET_METADATA_PREFIX) {
          $name_metadata = substr($name, drupal_strlen(CSA_ASSET_METADATA_PREFIX));

          $name_metadata = explode('_', $name_metadata);
          $key = array_pop($name_metadata);// remove key.

          $value = trim($value);

          if ($value != '') {
            $metadata[implode('_', $name_metadata)][$name] = $value;
          }
        }
      }

      _csa_asset_metadata_validate($metadata);
      break;

    case CSA_ASSET_METADATA_CANCEL:
      form_set_error(NULL, '', TRUE);
      drupal_get_messages('error', TRUE);
      $_REQUEST['destination'] = strtr('csa/!box/asset/!asset_id/!offset/!mode', array('!box' => rawurlencode(arg(1)), '!asset_id' => rawurlencode($values['asset_id']), '!offset' => rawurlencode($values['offset']), '!mode' => rawurlencode($values['mode'])));
      drupal_goto();
      break;
  }
}

/**
 * Submit function for access form.
 */
function _csa_asset_metadata_form_submit($form, &$form_state) {

  $values = $form_state['values'];

  $no_errors = TRUE;

  switch ($values['op']) {
    case t(CSA_ASSET_METADATA_SAVE_METADATA_AND_NEXT):
    case t(CSA_ASSET_METADATA_SAVE_METADATA):
      $metadata = array();
      foreach ($values as $name => $value) {
        // Name is in format of [CSA_ASSET_METADATA_PREFIX] [name] [_#]
        if (substr($name, 0, drupal_strlen(CSA_ASSET_METADATA_PREFIX)) ==  CSA_ASSET_METADATA_PREFIX) {
          $name = substr($name, drupal_strlen(CSA_ASSET_METADATA_PREFIX));

          $name = explode('_', $name);
          $key = array_pop($name);// remove key.

          $value = trim($value);

          if ($value != '') {
            $metadata[implode('_', $name)][] = $value;
          }
        }
      }

      // Save it.
      $no_errors = _csa_asset_metadata_save($values['asset_id'], $metadata, array('set_message' => TRUE));

      if (!$no_errors) {
        $form_state['rebuild'] = TRUE;
      }

      // Log it.
      _csa_log_info('Saved metadata for asset with ID @asset_id', array('@asset_id' => $values['asset_id']), url('csa/asset/' . $values['asset_id']));
      break;

    case t(CSA_ASSET_METADATA_ADD_FIELD_OPTION):
      $form_state['storage']['metadata_group_add'] = array_merge(isset($form_state['storage']['metadata_group_add']) ? $form_state['storage']['metadata_group_add'] : array(), $values['metadata_group_add']);

      // Prevent losing our posted data.
      $form_state['rebuild'] = TRUE;
      break;
  }

  if ($no_errors && $values['op'] == t(CSA_ASSET_METADATA_SAVE_METADATA_AND_NEXT)) {
    unset($form_state['storage']);
    $_REQUEST['destination'] = $values['asset_next_url'];
  }

  // Done, go back to details of the asset.
  if ($no_errors && $values['op'] == t(CSA_ASSET_METADATA_SAVE_METADATA)) {
    unset($form_state['storage']);
    $_REQUEST['destination'] = strtr('csa/!box/asset/!asset_id/!offset/!mode', array('!box' => rawurlencode(arg(1)), '!asset_id' => rawurlencode($values['asset_id']), '!offset' => rawurlencode($values['offset']), '!mode' => rawurlencode($values['mode'])));
  }
}

/**
 * Validate input.
 */
function _csa_asset_metadata_validate($metadata_to_validate, $options = array()) {

  // Metadata.
  $metadata = _csa_asset_metadata_list_misc();

  // Unset options for metadata that are double and are not allowed to be double.
  $metadata_options = $metadata['metadata_options'];

  foreach ($metadata['metadata'] as $group_name => $metadata_group) {
    foreach ($metadata_group as $name => $property) {

      $prop_id = $property['prop_id'];

      // Validate?
      if (isset($metadata_to_validate[$name]) && !empty($metadata['metadata_propid_to_type'][$prop_id])) {

        // Get title.
        $title = $metadata['metadata_propid_to_title'][$prop_id];

        // Get type.
        $type = $metadata['metadata_propid_to_type'][$prop_id];

        foreach ($metadata_to_validate[$name] as $name_form => $value) {
          switch ($type) {
            case 'CHAR':
              break;

            case 'INT':
              if (!is_numeric($value)) {
                form_set_error($name_form, t("Value for '@title' must be numeric", array('@title' => $title)));
              }
              break;

            case 'DATETIME':
              if (trim($value) == '' || _csa_check_datestamp($value) === FALSE) {
                form_set_error($name_form, t("Invalid date time value for '@title', must be in format of 'YYYY-MM-DD HH-MM-SS' or 'YYYY-MM-DD'.", array('@title' => $title)));
              }
              break;
          }
        }
      }
    }
  }
}

/**
 * Save metadata for given asset.
 *
 * @param array $metadata
 */
function _csa_asset_metadata_save($asset_id, $metadata, $options = array()) {

  $options += array(
    'action' => 'replace',
    'set_message' => FALSE,
    'fatal' => FALSE,
  );

  // Get the asset.
  $asset = _csa_asset_get($asset_id, array('fatal' => $options['fatal']));

  // Copy it.
  $data = $metadata;
  $data['action'] = $options['action'];
  $data['user_id'] = _csa_asset_owner_get($asset);

  $request_options = array(
    'data' => $data,
    'fatal' => $options['fatal'],
  );

  // Setup logging.
  $message = 'Unable to save metadata for asset with ID @asset_id';
  $variables = array('@asset_id' => $asset_id);

  try {
    // Do REST call.
    $response = _csa_mediamosa_request_post('asset/' . rawurlencode($asset_id) . '/metadata', $request_options);
  }
  catch (Exception $e) {
    _csa_log_rest_response(FALSE, $message, $variables);
    throw $e;
  }

  // Set message.
  if (mediamosa_connector::check_result($response)) {
    if ($options['set_message']) {
      drupal_set_message(t('Metadata saved for @title.', array('@title' => _csa_asset_get_title($asset))));
    }
  }

  // Log it (if the request failed).
  return _csa_log_rest_response($response, $message, $variables);
}
