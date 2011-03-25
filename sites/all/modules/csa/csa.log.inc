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
 * This module contains logging functions.
 */

/**
 * Log a system message.
 *
 * @param $message
 *   The message to store in the log. See t() for documentation
 *   on how $message and $variables interact. Keep $message
 *   translatable by not concatenating dynamic values into it!
 * @param $variables
 *   Array of variables to replace in the message on display or
 *   NULL if message is already translated or not possible to
 *   translate.
 * @param $type
 *   The category to which this message belongs.
 * @param $link
 *   A link to associate with the message.
 * @param $severity
 *   The severity of the message, as per RFC 3164
 *
 * @see watchdog()
 */
function _csa_log($message, $variables = array(), $type = '', $link = NULL, $severity = WATCHDOG_NOTICE) {
  // Call watchdog.
  watchdog('CSA' . (empty($type) ? '' : ' - ' . $type), $message, $variables, $severity, $link);
}

/**
 * Gives pretty log entries.
 *
 * @param any $data
 *  Just provide any object type.
 * @param string $type
 *   The category to which this message belongs.
 * @param $link
 *   A link to associate with the message.
 * @param string $severity
 *   The severity of the message, as per RFC 3164
 */
function _csa_log_export($data, $type = '', $link = NULL,  $severity = WATCHDOG_NOTICE) {
  _csa_log(highlight_string(var_export($data, TRUE), TRUE), array(), $type, $link, $severity);
}


/**
 * Log a system message.
 *
 * @param $message
 *   The message to store in the log. See t() for documentation
 *   on how $message and $variables interact. Keep $message
 *   translatable by not concatenating dynamic values into it!
 * @param $variables
 *   Array of variables to replace in the message on display or
 *   NULL if message is already translated or not possible to
 *   translate.
 * @param $type
 *   The category to which this message belongs.
 * @param $link
 *   A link to associate with the message.
 *
 * @see _csa_log()
 */
function _csa_log_debug($message, $variables = array(), $type = '', $link = NULL) {
  $variables['!csa_debug_dump'] = 'Trace dump: ' . print_r(debug_backtrace(), TRUE);
  $message .= "\n!csa_debug_dump";
  _csa_log($message, $variables, $type, $link , WATCHDOG_DEBUG);
}

/**
 * Gives pretty log entries.
 *
 * @param any $data
 *  Just provide any object type.
 * @param string $type
 *   The category to which this message belongs.
 * @param $link
 *   A link to associate with the message.
 * @param string $severity
 *   The severity of the message, as per RFC 3164
 */
function _csa_log_export_debug($data, $type = '', $link = NULL) {
  $message = '!csa_export_dump';
  $variables['!csa_export_dump'] = highlight_string(var_export($data, TRUE), TRUE);
  _csa_log_debug($message, $variables, $type, $link);
}

/**
 * Log a system message.
 *
 * @param $message
 *   The message to store in the log. See t() for documentation
 *   on how $message and $variables interact. Keep $message
 *   translatable by not concatenating dynamic values into it!
 * @param $variables
 *   Array of variables to replace in the message on display or
 *   NULL if message is already translated or not possible to
 *   translate.
 * @param $link
 *   A link to associate with the message.
 * @param $type
 *   The category to which this message belongs.
 *
 * @see _csa_log()
 */
function _csa_log_info($message, $variables = array(), $link = NULL, $type = '') {
  _csa_log($message, $variables, $type, $link, WATCHDOG_INFO);
}

/**
 * Log a system message.
 *
 * @param $message
 *   The message to store in the log. See t() for documentation
 *   on how $message and $variables interact. Keep $message
 *   translatable by not concatenating dynamic values into it!
 * @param $variables
 *   Array of variables to replace in the message on display or
 *   NULL if message is already translated or not possible to
 *   translate.
 * @param $type
 *   The category to which this message belongs.
 * @param $link
 *   A link to associate with the message.
 *
 * @see _csa_log()
 */
function _csa_log_warning($message, $variables = array(), $type = '', $link = NULL) {
  _csa_log($message, $variables, $type, $link, WATCHDOG_WARNING);
}

/**
 * Log a system message.
 *
 * @param $message
 *   The message to store in the log. See t() for documentation
 *   on how $message and $variables interact. Keep $message
 *   translatable by not concatenating dynamic values into it!
 * @param $variables
 *   Array of variables to replace in the message on display or
 *   NULL if message is already translated or not possible to
 *   translate.
 * @param $type
 *   The category to which this message belongs.
 * @param $link
 *   A link to associate with the message.
 *
 * @see _csa_log()
 */
function _csa_log_critical($message, $variables = array(), $type = '', $link = NULL) {
  drupal_set_message(t('CSA: Critical message !message', array('!message' => t($message, $variables))));
  _csa_log($message, $variables, $type, $link, WATCHDOG_CRITICAL);
}

/**
 * Gives pretty log entries.
 *
 * @param any $data
 *  Just provide any object type.
 * @param string $type
 *   The category to which this message belongs.
 * @param $link
 *   A link to associate with the message.
 * @param string $severity
 *   The severity of the message, as per RFC 3164
 */
function _csa_log_export_warning($data, $type = '', $link = NULL) {
  $message = '!csa_export_dump';
  $variables['!csa_export_dump'] = highlight_string(var_export($data, TRUE), TRUE);
  _csa_log_warning($message, $variables, $type, $link);
}

/**
 * Log a REST response.
 *
 * @param string $response
 *   The result reponse object.
 * @param unknown_type $message
 *   Custom message.
 * @param array $variables
 *   The variable array.
 *
 * @return:
 *   Return TRUE / FALSE if error was detected.
 */
function _csa_log_rest_response($response, $message = '', $variables = array(), $drupal_set_message = TRUE) {
  // Need to log?
  if (mediamosa_connector::check_result($response)) {
    return TRUE;
  }

  $request_uri = empty($response->xml->header->request_uri) ? '' : $response->xml->header->request_uri;
  $variables['@request_result_description'] = empty($response->xml->header->request_result_description) ? '' : (string)$response->xml->header->request_result_description;

  if (empty($message)) {
    $message = 'Unable to execute REST @request_uri call';
    $variables['@request_uri'] = $request_uri;
  }

  // Log.
  _csa_log_warning($message . empty($variables['@request_result_description']) ? '' : ", reason given; '@request_result_description'", $variables);

  if ($drupal_set_message) {
    drupal_set_message(t($message . (empty($variables['@request_result_description']) ? '' : ", reason given; '@request_result_description'"), $variables), 'error');
  }

  return FALSE;
}
