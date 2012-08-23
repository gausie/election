<?php
/**
 * @file
 * Election hooks for the Election STV module.
 */

/**
 * Implements hook_election_type_info().
 */
function election_stv_election_type_info() {
  return array(
    'stv' => array(
      'name' => t('STV election'),
      'description' => t('A Single Transferable Vote election where people can rank candidates in order of preference, for one or more positions.'),
      'post name' => t('position'),
      'has candidates' => TRUE,
      'vote form' => 'election_stv_vote_form',
      'save votes' => 'election_stv_save_votes',
      'results page' => 'election_stv_results_count_page',
      'results download form' => 'election_stv_results_download_form',
    ),
  );
}