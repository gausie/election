<?php
/**
 * @file
 * This file contains no working PHP code; it exists to provide additional
 * documentation for doxygen as well as to document hooks in the standard
 * Drupal manner.
 */

/**
 * Help determine whether the election post has enough candidates for voting.
 *
 * @see election_candidate_post_has_enough()
 *
 * @param bool &$enough
 *   Whether the post has enough candidates for voting to be possible.
 * @param stdClass $post
 *   The election post object.
 * @param int $num_candidates
 *   The number of published, 'hopeful' candidates for this post.
 */
function hook_election_candidate_post_has_enough_alter(&$enough, $post, $num_candidates) {
  // Example: always say that there are 'enough' candidates for voting, if the
  // post allows for "write-in" candidates.
  if (!empty($post->settings['write_in']) && module_exists('election_write_in')) {
    $enough = TRUE;
  }
}
