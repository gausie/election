<?php
/**
 * @file
 * This file contains no working PHP code; it exists to provide additional
 * documentation for doxygen as well as to document hooks in the standard
 * Drupal manner.
 */

/**
 * Alter the form elements provided for specifying a write-in candidate.
 *
 * @param array &$parent_element
 *   The Form API parent element, to which write-in candidate elements are
 *   added.
 * @param stdClass $post
 *   The election post object.
 */
function hook_election_write_in_elements_alter(&$parent_element, $post) {
  // Add an e-mail address field.
  $parent_element['mail'] = array(
    '#type' => module_exists('elements') ? 'emailfield' : 'textfield',
    '#title' => t('E-mail address'),
  );
}
