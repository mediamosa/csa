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
 * Opposite of parse_url.
 *
 * @param $urls
 * @param $return_parts
 *  Return the array with host and uri.
 */
function _csa_build_url($urls, $return_parts = FALSE) {

  $result = array(
    'host' => array($urls['scheme'] . '://'),
    'uri' => array(),
  );

  // Setup default.
  $urls += array(
    'user' => '',
    'pass' => '',
    'port' => 0,
    'path' => '',
    'query' => '',
    'fragment' => '',
  );

  // Add user : pass
  if ($urls['user'] != '' || $urls['pass'] != '') {
    $result['host'][] = $urls['user'];
    $result['host'][] = ':';
    $result['host'][] = $urls['pass'];
    $result['host'][] = '@';
  }

  // Host.
  $result['host'][] = trim($urls['host'], '/');

  // Optional port
  if ($urls['port']) {
    $result['host'][] = ':' . $urls['port'];
  }

  // Path.
  if ($urls['path'] != '') {
    $result['uri'][] = '/' . trim($urls['path'], '/');
  }

  // Query.
  if ($urls['query'] != '') {
    $result['uri'][] = '?' . $urls['query'];
  }

  // Fragment.
  if ($urls['fragment'] != '') {
    $result['uri'][] = $urls['fragment'];
  }

  // Now rebuild url.
  return $return_parts ? array('host' => implode('', $result['host']), 'uri' => implode('', $result['uri'])) : (implode('', $result['host']) . implode('', $result['uri']));
}

/**
 * Wrapper around http_build_query().
 *
 * @param array $query
 */
function _csa_http_build_query($query) {
  return strtr(http_build_query($query, '', '&'), array('%5B' => '[', '%5D' => ']', '%5b' => '[', '%5d' => ']'));
}

/**
 * Opposide of http_build_query()
 *
 * @param $query_str
 */
function _csa_parse_query($query_str) {
  // Split the url.
  $query = array();

  // Split values.
  foreach (explode('&', $query_str) as $valuepair) {
    if (strpos($valuepair, '=')) {
      list($name, $value) = explode('=', $valuepair, 2);
      $query[urldecode($name)][] = urldecode($value);
    }
    else {
      $query[urldecode($valuepair)][] = '';
    }
  }

  // Now make single item queries with 0 keys no array.
  foreach ($query as $name => $value) {
    if (count($value) == 1 && key($value) == 0) {
      $query[$name] = reset($value);
    }
  }

  return $query;
}

/**
 * Converts a PHP variable into its Javascript equivalent because the drupal 6
 * version is broken with its '&' to \x26amp; encoding.
 */
function _csa_json_encode($var) {
  // json_encode() does not escape <, > and &, so we do it with str_replace().
  return str_replace(array('<', '>', '&'), array('\u003c', '\u003e', '\u0026'), json_encode($var));
}

/**
 * Converts an HTML-safe JSON string into its PHP equivalent.
 */
function _csa_json_decode($var) {
  return json_decode($var, TRUE);
}

/**
 * Return data in JSON format.
 *
 * This function should be used for JavaScript callback functions returning
 * data in JSON format. It sets the header for JavaScript output.
 *
 * @param $var
 *   (optional) If set, the variable will be converted to JSON and output.
 */
function _csa_json_output($var = NULL) {
  // We are returning JSON, so tell the browser.
  drupal_set_header('Content-Type', 'application/json');

  if (isset($var)) {
    echo _csa_json_encode($var);
  }
}
