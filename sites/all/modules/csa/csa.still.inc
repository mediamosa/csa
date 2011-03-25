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
 * This include contains all code related to helper functions for stills.
 */

// Default settings.

define('CSA_STILL_TYPE_NONE', 'NONE');
define('CSA_STILL_TYPE_SECOND', 'SECOND');
define('CSA_STILL_TYPE_NORMAL', 'NORMAL');
define('CSA_STILL_TYPE_SCENE', 'SCENE');

// Still per mediafile
define('CSA_STILL_PER_MEDIAFILE', 3);
// Still on every second
define('CSA_STILL_EVERY_SECOND', 5);
// Start time
define('CSA_STILL_START_TIME', 0);
// End frame
define('CSA_STILL_END_TIME', 9999999);
// Size
define('CSA_STILL_SIZE', '352x288');
// H padding
define('CSA_STILL_H_PADDING', 0);
// V padding
define('CSA_STILL_V_PADDING', 0);

// Submit button text.
define('CSA_SUBMIT_TEXT_GENERATE_STILL', 'Generate');
define('CSA_SUBMIT_TEXT_GENERATE_STILL_CANCEL', 'Cancel');

/**
 * Generate still for mediafile.
 *
 * There is no version for /asset/$asset_id/still/create because its exactly
 * the same.
 *
 * @param $mediafile_id
 *  The mediafile ID.
 * @param $options
 */
function _csa_still_create($mediafile_id, $user_id, $options = array()) {

  $options += array(
    'fatal' => FALSE,
    'still_type' => CSA_STILL_TYPE_NONE,
    'still_per_mediafile' => CSA_STILL_PER_MEDIAFILE,
    'still_every_second'=> CSA_STILL_EVERY_SECOND,
    'start_time' => CSA_STILL_START_TIME,
    'end_time' => CSA_STILL_END_TIME,
    'size' => CSA_STILL_SIZE,
    'h_padding' => CSA_STILL_H_PADDING,
    'v_padding' => CSA_STILL_V_PADDING,
  );

  $request_options = array(
    'user_id' => $user_id,
    'fatal' => $options['fatal'],
  );

  // only these settings.
  foreach (array('still_type', 'still_per_mediafile', 'still_every_second', 'start_time', 'end_time', 'size', 'h_padding', 'v_padding') as $name) {
    if (isset($options[$name])) {
      $request_options['data'][$name] = $options[$name];
    }
  }

  // some params are not used with still types.
  switch ($options['still_type']) {

  }

  // Setup logging.
  $message = 'Unable to create still for mediafile with ID @mediafile_id';
  $variables = array('@mediafile_id' => $mediafile_id);

  try {
    // Do rest call.
    $response = _csa_mediamosa_request_post('mediafile/' . rawurlencode($mediafile_id) . '/still/create', $request_options);
  }
  catch (Exception $e) {
    _csa_log_rest_response(FALSE, $message, $variables);
    throw $e;
  }

  // Log it (if the request failed).
  return _csa_log_rest_response($response, $message, $variables);
}

/**
 * Stil generation form.
 *
 * @param array $form_state
 * @param array $options
 */
function _csa_still_create_form($form_state, $options = array()) {

  // JS.
  drupal_add_js(drupal_get_path('module', 'csa') . '/csa.js');

  // Get possible values.
  $values = empty($form_state['values']) ? array() : $form_state['values'];

  // Get default still type.
  $still_type = empty($values['still_type']) ? CSA_STILL_TYPE_NONE : $values['still_type'];

  $options += array(
    'hidden' => array(),
  );

  $form['still'] = array(
    '#type' => 'fieldset',
    '#title' => 'Still generate parameters',
    '#collapsible' => TRUE,
    '#collapsed' => FALSE
  );

  $form['still']['still_type'] = array(
    '#title' => t('Select your still type'),
    '#type' => 'select',
    '#options' => array(CSA_STILL_TYPE_NONE => 'Default', CSA_STILL_TYPE_NORMAL => '# of stills per mediafile', CSA_STILL_TYPE_SECOND => 'A still every # of seconds', CSA_STILL_TYPE_SCENE => 'A still for each scene'),
    '#size' => 1,
    '#required' => TRUE,
    '#default_value' => $still_type,
    '#attributes' => array('onchange' => "csa_still_hide_by_type(this);"),
  );

   // All
  $form['still']['size'] = array(
    '#title' => t('Size of still (in pixels, WidthxHeight)'),
    '#type' => 'textfield',
    '#size' => 15,
    '#default_value' => variable_get('csa_still_size', CSA_STILL_SIZE),
    '#required' => TRUE,
  );

   // All
  $form['still']['h_padding'] = array(
    '#title' => t('Horizontal padding (in pixels)'),
    '#type' => 'textfield',
    '#size' => 4,
    '#default_value' => variable_get('csa_still_h_padding', CSA_STILL_H_PADDING),
    '#required' => TRUE,
  );

   // All
  $form['still']['v_padding'] = array(
    '#title' => t('Vertical padding (in pixels)'),
    '#type' => 'textfield',
    '#size' => 4,
    '#default_value' => variable_get('csa_still_v_padding', CSA_STILL_V_PADDING),
    '#required' => TRUE,
  );

  // NORMAL.
  $form['still']['still_per_mediafile'] = array(
    '#prefix' => '<div id="show-csa-still-per-mediafile" style="display:' . ($still_type == CSA_STILL_TYPE_NORMAL ? 'block' : 'none') . ';">',
    '#suffix' => '</div>',
    '#title' => t('# of stills per mediafile'),
    '#type' => 'textfield',
    '#size' => 3,
    '#default_value' => variable_get('csa_still_per_mediafile', CSA_STILL_PER_MEDIAFILE),
  );

  // SECOND.
  $form['still']['still_every_second'] = array(
    '#prefix' => '<div id="show-csa-still-every-second" style="display:' . ($still_type == CSA_STILL_TYPE_SECOND ? 'block' : 'none') . ';">',
    '#suffix' => '</div>',
    '#title' => t('One still every # of seconds'),
    '#type' => 'textfield',
    '#size' => 6,
    '#default_value' => variable_get('csa_still_every_second', CSA_STILL_EVERY_SECOND),
  );

  // NORMAL / SECOND.
  $form['still']['start_time'] = array(
    '#prefix' => '<div id="show-csa-start-time" style="display:' . ($still_type == CSA_STILL_TYPE_NORMAL || $still_type == CSA_STILL_TYPE_SECOND ? 'block' : 'none') . ';">',
    '#suffix' => '</div>',
    '#title' => t('Start time in seconds'),
    '#type' => 'textfield',
    '#size' => 7,
    '#default_value' => variable_get('csa_still_start_time', CSA_STILL_START_TIME),
  );

  // NORMAL / SECOND.
  $form['still']['end_time'] = array(
    '#prefix' => '<div id="show-csa-end-time" style="display:' . ($still_type == CSA_STILL_TYPE_NORMAL || $still_type == CSA_STILL_TYPE_SECOND ? 'block' : 'none') . ';">',
    '#suffix' => '</div>',
    '#title' => t('End time in seconds'),
    '#type' => 'textfield',
    '#size' => 7,
    '#default_value' => variable_get('csa_still_end_time', CSA_STILL_END_TIME),
  );

  // Submit buttons
  $form['submit']['generate'] = array(
    '#type' => 'submit',
    '#value' => t(CSA_SUBMIT_TEXT_GENERATE_STILL),
  );

  $form['submit']['cancel'] = array(
    '#type' => 'button',
    '#value' => t(CSA_SUBMIT_TEXT_GENERATE_STILL_CANCEL),
  );

  // Save the contents of options.
  foreach ($options['hidden'] as $name => $value) {
    $form['hidden'][$name] = array(
      '#type' => 'hidden',
      '#value' => $value,
    );
  }

  return $form;
}
