<?php
/**
 * @file
 * Filter by candidate status.
 */

class election_candidate_views_handler_filter_cstatus extends views_handler_filter_in_operator {

  /**
   * Overrides parent::get_value_options().
   */
  public function get_value_options() {
    $this->value_options = _election_candidate_get_cstatus_options();
    return $this->value_options;
  }

}