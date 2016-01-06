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
    '#value' => t('Add items to Coursepack'),
  );
  return $form;
}

function agile_coursepack_search_form_submit($form, &$form_state) {
  if (!isset($_GET['hidden_filter'])) {
    $_GET['hidden_filter'] = array('');
  }
  else {
    $_GET['hidden_filter'][0] .= ' || ';
  }
  $content_models = array(
    'info:fedora/islandora:sp_videoCModel',
    'info:fedora/islandora:sp-audioCModel',
    'info:fedora/islandora:sp_basic_image',
    'info:fedora/islandora:sp_large_image_cmodel',
    'info:fedora/islandora:sp_document',
  );
  $content_models_string = implode('" || "', $content_models);
  $_GET['hidden_filter'][0] .= 'RELS_EXT_hasModel_uri_ms:("' . $content_models_string . '")';
  module_load_include('inc', 'agile_coursepack', 'includes/utilities');
  $coursepack_pid = agile_coursepack_get_pid_from_url();
  $form_state['coursepack_pid'] = $coursepack_pid;
  $form_state['redirect'] = "islandora/object/$coursepack_pid";
  $search_string = str_replace('/', '~slsh~', $form_state['values']['agile_coursepack_search_query']);
  if (empty($search_string)) {
    $search_string = '%20';
  }
  $options = array('query' => array('type' => 'dismax'));
  $options['query']['display'] = 'coursepack';
  $options['query']['pid'] = $form_state['coursepack_pid'];
  $options['query']['hidden_filter'][0] = 'RELS_EXT_hasModel_uri_ms:("' . $content_models_string . '")';
  drupal_goto('islandora/search/' . $search_string, $options);
}
