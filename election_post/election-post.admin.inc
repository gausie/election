<?php
/**
 * @file
 * Administration page callbacks for the Election Post module.
 */

/**
 * Form builder function for adding or editing an election post.
 *
 * @param array $form
 * @param array &$form_state
 * @param stdClass $post
 * @param stdClass $election
 *   Optional: the election the post belongs to.
 *
 * @return array
 *   Render array.
 */
function election_post_form($form, &$form_state, stdClass $post, $election = NULL) {

  if (empty($election)) {
    $election = election_load($post->election_id);
  }

  $post->is_new = empty($post->post_id);

  if ($post->is_new) {
    $breadcrumb = _election_build_breadcrumb(
      $election,
      '_POSTS_'
    );
  }
  else {
    $breadcrumb = _election_build_breadcrumb(
      $election,
      '_POSTS_',
      l($post->title, election_post_uri_path($post))
    );
  }

  drupal_set_breadcrumb($breadcrumb);

  $form['#id'] = 'election-post-form';
  $form_state['post'] = $post;
  $form_state['election'] = $election;

  $form['title'] = array(
    '#type' => 'textfield',
    '#title' => t('Title'),
    '#default_value' => isset($post->title)? $post->title : '',
    '#required' => TRUE,
    '#weight' => -100,
  );

  $type = _election_type_get_info($election->type);
  $post_key = $type['post key'];
  $post_form_function = 'election_post_' . $post_key . '_form';
  if (function_exists($post_form_function)) {
    $form = $post_form_function($form, $form_state, $post, $election);
  }

  $form['role_access'] = array(
    '#type' => 'fieldset',
    '#title' => t('Role eligibility'),
    '#collapsible' => FALSE,
    '#weight' => 10,
  );
  $roles_that_can_vote_global = user_roles(FALSE, 'vote in elections');
  $form['role_access']['rids'] = array(
    '#type' => 'checkboxes',
    '#title' => t('Roles eligible to vote'),
    '#options' => $roles_that_can_vote_global,
    '#description' => t('Select the roles eligible to vote in this election. There may be other restrictions. Only roles with the permission \'!election_perm\' are listed here.', array('!election_perm' => l('vote in elections', 'admin/people/permissions', array('fragment' => 'module-election', 'attributes' => array('target' => '_blank'))))),
    '#default_value' => (array) $post->rids,
  );

  if ($post->is_new) {
    $form['role_access']['rids']['#default_value'] = array_keys($roles_that_can_vote_global);
  }

  $form['voting'] = array(
    '#type' => 'fieldset',
    '#title' => t('Voting'),
    '#collapsible' => TRUE,
    '#collapsed' => (!isset($post->vstatus_inheritance) || $post->vstatus_inheritance == ELECTION_POST_STATUS_INHERIT),
    '#weight' => 98,
  );
  $form['voting']['vstatus_inheritance'] = array(
    '#type' => 'radios',
    '#title' => t('Voting status'),
    '#default_value' => isset($post->vstatus_inheritance)? $post->vstatus_inheritance : ELECTION_POST_STATUS_INHERIT,
    '#options' => array(
      ELECTION_POST_STATUS_INHERIT => t('Inherited (currently: %status)', array('%status' => strip_tags(_election_format_status($election, 'v')))),
      ELECTION_POST_STATUS_CLOSED => t('Closed'),
    ),
  );

  $form['buttons'] = array(
    '#type' => 'actions',
    '#weight' => 100,
  );
  $form['buttons']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
    '#submit' => array('election_post_form_submit'),
  );
  if (!$post->is_new) {
    $form['buttons']['delete'] = array(
      '#type' => 'submit',
      '#value' => t('Delete'),
      '#submit' => array('election_post_form_delete_submit'),
      '#access' => entity_access('update', 'election', $election), // deleting a post is really 'updating' an election
    );
  }

  $form['#validate'][] = 'election_post_form_validate';

  // Add custom fields from the Field system.
  field_attach_form('election_post', $post, $form, $form_state);

  return $form;
}

/**
 * Partial form builder function for [referendum] motions.
 *
 * @see election_post_form()
 *
 * @param array $form
 * @param array &$form_state
 * @param stdClass $election
 * @param stdClass $post
 *
 * @return array
 */
function election_post_motion_form($form, &$form_state, $post, $election = NULL) {

  $form['allow_abstention'] = array(
    '#type' => 'select',
    '#title' => t('Allow abstention'),
    '#description' => t(
      'Select whether voters would be able to select @abstain as an option alongside @yes and @no.',
      array(
        '@abstain' => t('Abstain'),
        '@yes' => t('Yes'),
        '@no' => t('No'),
      )
    ),
    '#default_value' => isset($post->allow_abstention)? $post->allow_abstention : 1,
    '#options' => array(
      1 => t('Allow abstention'),
      0 => t('Do not allow abstention'),
    ),
    '#weight' => -10,
  );

  return $form;
}

/**
 * Partial form builder function for [electoral] positions.
 *
 * @see election_post_form()
 *
 * @param array $form
 * @param array &$form_state
 * @param stdClass $post
 * @param stdClass $election
 *
 * @return array
 */
function election_post_position_form($form, &$form_state, $post, $election = NULL) {

  if (empty($election)) {
    $election = election_load($post->election_id);
  }

  $form['exclusive'] = array(
    '#type' => 'checkbox',
    '#title' => t('Exclusive'),
    '#description' => t('A candidate can only stand for one exclusive position per election.<br /><strong>N.B. if anonymous users are given permission to submit nominations, exclusivity cannot be enforced.</strong>'),
    '#default_value' => isset($post->exclusive)? $post->exclusive : 1,
    '#weight' => -30,
  );

  $form['vacancy_count'] = array(
    '#type' => 'textfield',
    '#title' => t('Number of vacancies'),
    '#default_value' => isset($post->vacancy_count)? $post->vacancy_count : 1,
    '#size' => 2,
    '#maxlength' => 2,
    '#required' => TRUE,
    '#weight' => -20,
  );

  $form['use_ron'] = array(
    '#type' => 'select',
    '#title' => t('RON (Re-Open Nominations)'),
    '#default_value' => isset($post->use_ron)? $post->use_ron : 1,
    '#options' => array(
      1 => t('Include RON as a candidate'),
      0 => t('Do not include RON'),
    ),
    '#weight' => -10,
  );

  if (!variable_get('election_allow_multi_vacancy_ron', TRUE)) { // @todo settings form
    $form['use_ron']['#states'] = array(
      'visible' => array(
        ':input[name=vacancy_count]' => array('value' => '1'),
      ),
    );
    $form['use_ron']['#description'] = t('RON can only be included for single-vacancy positions.');
  }

  $form['nominations'] = array(
    '#type' => 'fieldset',
    '#title' => t('Nominations'),
    '#collapsible' => TRUE,
    '#collapsed' => (!isset($post->nstatus_inheritance) || $post->nstatus_inheritance == ELECTION_POST_STATUS_INHERIT),
    '#weight' => 97,
  );

  $form['nominations']['nstatus_inheritance'] = array(
    '#type' => 'radios',
    '#title' => t('Nominations status'),
    '#default_value' => isset($post->nstatus_inheritance)? $post->nstatus_inheritance : ELECTION_POST_STATUS_INHERIT,
    '#options' => array(
      ELECTION_POST_STATUS_INHERIT => t('Inherited (currently: %status)', array('%status' => strip_tags(_election_format_status($election, 'n')))),
      ELECTION_POST_STATUS_CLOSED => t('Closed'),
    ),
  );

  return $form;
}

/**
 * Form submit function for the Delete button in the form election_post_form.
 *
 * @param array $form
 * @param array &$form_state
 * @return void
 */
function election_post_form_delete_submit($form, &$form_state) {
  $destination = array();
  if (isset($_GET['destination'])) {
    $destination = drupal_get_destination();
    unset($_GET['destination']);
  }
  $post = $form_state['post'];
  $election = $form_state['election'];
  $form_state['redirect'] = array(
    election_post_uri_path($post) . '/delete',
    array('query' => $destination),
  );
}


/**
 * Form validate function for the form election_post_form.
 *
 * @param array $form
 * @param array &$form_state
 * @return void
 */
function election_post_form_validate($form, &$form_state) {

  $post = $form_state['post'];

  if ($post->type != 'motion' && $form_state['values']['use_ron'] == 1 && $form_state['values']['vacancy_count'] > 1 && !variable_get('election_allow_multi_vacancy_ron', TRUE)) {
    form_set_error('use_ron', t('RON can only be included for single-vacancy positions.'));
  }

  field_attach_form_validate('election_post', $form_state['post'], $form, $form_state);

}

/**
 * Form submit function for the Save button in the form election_post_form.
 *
 * @param array $form
 * @param array &$form_state
 * @return void
 */
function election_post_form_submit($form, &$form_state) {

  $post = $form_state['post'];
  $election = $form_state['election'];

  if (empty($post->uid)) {
    global $user;
    $post->uid = $user->uid;
  }

  foreach ($post as $property => $value) {
    if (isset($form_state['values'][$property])) {
      $post->$property = $form_state['values'][$property];
    }
  }

  if (empty($form_state['values']['exclusive'])) {
    $post->exclusive = 0;
  }

  field_attach_submit('election_post', $post, $form, $form_state);

  // Save the post.
  election_post_save($post);

  // Notify the user that the post was saved.
  drupal_set_message(
    t('The @post_name "@title" was saved.', array('@post_name' => _election_get_posts_name($election->type), '@title' => $post->title))
  );

  if (empty($_GET['destination'])) {
    $form_state['redirect'] = election_uri_path($election) . '/posts';
  }

}

/**
 * Form builder function to confirm the delete action for an election post.
 * See http://api.drupal.org/api/drupal/modules--system--system.module/function/confirm_form.
 *
 * @param array $form
 * @param array &$form_state
 * @param stdClass $election
 * @param stdClass $post
 *
 * @return array
 *   Render array.
 */
function election_post_form_delete_confirm($form, &$form_state, $post) {

  $election = election_load($post->election_id);

  drupal_set_breadcrumb(
    _election_build_breadcrumb($election, '_POSTS_', l($post->title, election_post_uri_path($post)))
  );

  $form['election_id'] = array(
    '#type' => 'value',
    '#value' => $election->election_id,
  );
  $form['post_id'] = array(
    '#type' => 'value',
    '#value' => $post->post_id,
  );

  return confirm_form(
    $form,
    t(
      'Are you sure you want to delete the @post_name %post_title?',
      array(
        '@post_name' => _election_get_posts_name($election->type),
        '%post_title' => $post->title,
      )
    ),
    election_post_uri_path($post),
    t('This action cannot be undone.'),
    t('Delete'),
    t('Cancel')
  );
}

/**
 * Form submit function for the Delete button in the form election_post_form_delete_confirm.
 *
 * @param array $form
 * @param array &$form_state
 * @return void
 */
function election_post_form_delete_confirm_submit($form, &$form_state) {
  if ($form_state['values']['confirm']) {

    $election = election_load($form_state['values']['election_id']);

    $post = election_post_load($form_state['values']['post_id']);

    // Delete the post.
    $delete = election_post_delete($post->post_id);

    $post_name = _election_get_posts_name($election->type);

    if ($delete) {
      watchdog('election_post', 'Deleted @post_name %title', array('@post_name' => $post_name, '%title' => $post->title));
    }

    if ($delete) {
      drupal_set_message(
        t('The @post_name %title has been deleted.', array('@post_name' => $post_name, '%title' => $post->title))
      );
    }
    else {
      drupal_set_message(
        t('Failed to delete the @post_name %title.', array('@post_name' => $post_name, '%title' => $post->title)),
        'error'
      );
    }

  }

  if (empty($_GET['destination'])) {
    $form_state['redirect'] = election_uri_path($election) . '/posts';
  }

}

/**
 * Form builder function for the administrative list of election posts, path:
 * election/%election/posts/manage.
 *
 * @param array $form
 * @param array $form_state
 * @param stdClass $election
 *
 * @return array
 *   Render array.
 */
function election_post_manage_form($form, $form_state, $election) {

  // @todo this should not be necessary
  $title = election_post_manage_page_title($election);
//  drupal_set_title($title, PASS_THROUGH);
  drupal_set_breadcrumb(
    _election_build_breadcrumb($election)
  );

  // Build the sortable table header.
  $header = array(
    'title' => array('data' => t('Title'), 'field' => 'ep.title', 'sort' => 'asc'),
    'changed' => array('data' => t('Last updated'), 'field' => 'ep.changed'),
    'operations' => array('data' => t('Operations')),
  );

  // Build the posts query, adding a pager and sorter.
  $query = db_select('election_post', 'ep')
    ->extend('PagerDefault')
    ->extend('TableSort')
    ->fields('ep', array('post_id'))
    ->condition('election_id', $election->election_id);

  $query->limit(50)->orderByHeader($header);

  // Get an array of post IDs
  $post_ids = $query->execute()->fetchCol();

  $posts = election_post_load_multiple($post_ids);

  $destination = drupal_get_destination();

  $rows = array();
  foreach ($posts as $id => $post) {

    $rows[$post->post_id] = array(
      'title' => l($post->title, election_post_uri_path($post)),
      'changed' => format_date($post->changed, 'short'),
      'operations' => array(),
    );

    $operations = array();
    if (entity_access('update', 'election', $election)) {
      $operations['edit'] = l(
        t('edit'),
        election_post_uri_path($post) . '/edit',
        array('query' => $destination)
      );
      $operations['delete'] = l(
        t('delete'),
        election_post_uri_path($post) . '/delete',
        array('query' => $destination)
      );
    }

    if (count($operations)) {
      // Render an unordered list of operations links.
      $rows[$post->post_id]['operations'] = array(
        'data' => array(
          '#theme' => 'item_list',
          '#items' => $operations,
          '#attributes' => array('class' => array('operations')),
        ),
      );
    }

  }

  $form['election_posts'] = array(
    '#theme' => 'table',
    '#header' => $header,
    '#rows' => $rows,
    '#empty' => t('No @posts_name available.', array('@posts_name' => _election_get_posts_name($election->type, TRUE))),
  );

  $form['pager'] = array('#markup' => theme('pager'));

  return $form;

}


/**
 * Page callback for editing an election post, path:
 * election-post/%election_post/edit.
 *
 * @param stdClass $post
 *
 * @return array
 *   Render array generated by Form API for the form: election_post_form.
 */
function election_post_page_edit($post) {
  $title = election_post_page_title($post);
  drupal_set_title(t('Edit: %title', array('%title' => truncate_utf8($title, 20, TRUE, TRUE))), PASS_THROUGH);

  $election = election_load($post->election_id);

  drupal_set_breadcrumb(
    _election_build_breadcrumb($election, '_POSTS_', l($post->title, election_post_uri_path($post)))
  );

  return drupal_get_form('election_post_form', $post, $election);
}

/**
 * Page callback for adding an election post, path:
 * election/%election/posts/add.
 *
 * @param stdClass $election
 *
 * @return array
 *   Render array generated by Form API for the form: election_post_form.
 */
function election_post_page_add($election) {

  drupal_set_breadcrumb(
    _election_build_breadcrumb($election, '_POSTS_')
  );

  if (!is_object($election)) {
    return MENU_NOT_FOUND;
  }

  $values = array();
  $values['election_id'] = $election->election_id;
  $type = _election_type_get_info($election->type);
  $values['type'] = $type['post key'];

  $post = entity_get_controller('election_post')->create($values);

  $post_name = 'post';
  if (!empty($type['post name'])) {
    $post_name = $type['post name'];
  }

  drupal_set_title(
    t('Add new @post_name to %election', array('%election' => $election->title, '@post_name' => $post_name)),
    PASS_THROUGH
  );

  return drupal_get_form('election_post_form', $post, $election);

}