<?php

/**
 * @file
 * Creates solr retrieval forms.
 */

/**
 * Islandora Solr simple search form.
 *
 * @param array $form
 *   An associative array containing form structure.
 * @param array $form_state
 *   An associative array containing form state.
 *
 * @return array
 *   An associative array containing the fully built form structure.
 */
function agile_coursepack_search_form($form, &$form_state) {

  $form['simple'] = array(
    '#type' => 'container',
    '#attributes' => array(
      'class' => array(
        'container-inline',
      ),
    ),
  );
  $form['simple']['agile_coursepack_search_query'] = array(
    '#size' => '15',
    '#type' => 'textfield',
    '#title' => '',
    '#default_value' => '',
  );
  $form['simple']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Find assets to add to this coursepack'),
  );
  return $form;
}

function agile_coursepack_search_form_submit($form, &$form_state) {
  module_load_include('inc', 'agile_coursepack', 'includes/utilities');
  $coursepack_pid = agile_coursepack_get_pid_from_url();
  $form_state['coursepack_pid'] = $coursepack_pid;
  $form_state['redirect'] = "islandora/object/$coursepack_pid";
  $search_string = str_replace('/', '~slsh~', $form_state['values']['agile_coursepack_search_query']);
  $options = array('query' => array('type' => 'dismax'));
  $options['query']['display'] = 'coursepack';
  $options['query']['pid'] = $form_state['coursepack_pid'];
  //return islandora_solr($search_string);
  drupal_goto('islandora/search/' . $search_string, $options);
}