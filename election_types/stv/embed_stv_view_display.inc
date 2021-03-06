<?php
/**
 * @file
 * STV-specific alterations to the 'election_posts' default View.
 *
 * The original election_posts View is provided by the Election Post module.
 *
 * This is part of an implementation of hook_views_default_views_alter().
 *
 * @see election_stv_views_default_views_alter()
 */

/* Display: STV election positions */
$handler = $view->new_display('embed', 'STV election positions', 'embed_stv');
$handler->display->display_options['defaults']['title'] = FALSE;
$handler->display->display_options['title'] = 'Positions';
$handler->display->display_options['defaults']['hide_admin_links'] = FALSE;
$handler->display->display_options['defaults']['empty'] = FALSE;
/* No results behavior: Global: Text area */
$handler->display->display_options['empty']['area']['id'] = 'area';
$handler->display->display_options['empty']['area']['table'] = 'views';
$handler->display->display_options['empty']['area']['field'] = 'area';
$handler->display->display_options['empty']['area']['label'] = 'No positions found';
$handler->display->display_options['empty']['area']['empty'] = TRUE;
$handler->display->display_options['empty']['area']['content'] = 'No positions found.';
$handler->display->display_options['empty']['area']['format'] = 'filtered_html';
$handler->display->display_options['defaults']['filter_groups'] = FALSE;
$handler->display->display_options['defaults']['filters'] = FALSE;
/* Filter criterion: Election post: Published or admin */
$handler->display->display_options['filters']['published_or_admin_1']['id'] = 'published_or_admin_1';
$handler->display->display_options['filters']['published_or_admin_1']['table'] = 'election_post';
$handler->display->display_options['filters']['published_or_admin_1']['field'] = 'published_or_admin';
/* Filter criterion: Election: Type */
$handler->display->display_options['filters']['type']['id'] = 'type';
$handler->display->display_options['filters']['type']['table'] = 'election';
$handler->display->display_options['filters']['type']['field'] = 'type';
$handler->display->display_options['filters']['type']['value'] = array(
  'stv' => 'stv',
);
