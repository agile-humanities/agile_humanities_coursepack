<?php

/**
 * @file
 * Contains methods to search solr and display results. Depends on
 * Apache_Solr_Php client.
 */

/**
 * Extention of IslandoraSolrResults to create an alternative display type.
 */
class IslandoraSolrResultsCoursepack extends IslandoraSolrResultsBookmark {

  /**
   * Constructor.
   */
  public function __construct() {
    parent::__construct();
  }



  /**
   * Form building method.
   *
   * @param array $form
   *   The Drupal form definition.
   * @param array $form_state
   *   The Drupal form state.
   * @param array $object_results
   *   The object results from the Islandora Solr processor created in the
   *   printResults function.
   *
   * @return array
   *   An array containing form structure.
   */
  public function formGet($form, &$form_state, $object_results) {
    module_load_include('inc', 'islandora_bookmark', 'includes/api');
    $form_state['islandora_solr_query_processor'] = $this->islandoraSolrQueryProcessor;
    $header = $this->getTableHeader();
    $rows = array();
    foreach ($object_results as $object_result) {
      $rows[$object_result['PID']] = $this->getTableRow($object_result);
    }
    if (count($rows) > 0) {
      $form = array_merge($this->getActions($form_state), $form);
    }
    $form['islandora_solr_select_table'] = array(
      '#header' => $header,
      '#type' => 'tableselect',
      '#options' => $rows,
      '#attributes' => array('class' => array('islandora-bookmark-solr-results')),
      '#empty' => t('No results found.'),
    );
    return $form;
  }

  /**
   * Get the header row to use in the tableselect.
   *
   * @return array
   *   An associative array as used as headers by theme_tableselect()/
   */
  protected function getTableHeader() {
    return array(
      'markup' => t("Title"),
    );
  }

  /**
   * Build a row for inclusion in the tableselect.
   *
   * @param array $object_result
   *   A result from the query processor.
   *
   * @return array
   *   An associative array of cells, keyed according to the header provided in
   *   $this->getTableHeader().
   */
  protected function getTableRow($object_result) {
    $doc = $object_result['solr_doc'];
    $solr_field = variable_get('islandora_solr_object_label_field', 'fgs_label_s');
    $markup = '';
    if (isset($doc[$solr_field])) {
      $markup = $doc[$solr_field];
    }
    else {
      $object = islandora_object_load($object_result['PID']);
      $markup = $object->label;
    }

    $object_url_info = array(
      'path' => $object_result['object_url'],
      'params' => $object_result['object_url_params'],
      'markup' => $markup,
    );

    return array(
      'markup' => islandora_bookmark_generate_markup($object_result['PID'], $object_url_info),
    );
  }

  /**
   * Get the "actions" section of the form.
   *
   * @param array $form_state
   *   The Drupal form state.
   *
   * @return array
   *   A form fragment containing a number of actions which might be performed
   *   on objects from the table select.
   *
   *   Submit buttons may have two additional properties:
   *   - action_validate: An array of validation methods to call, which should
   *     form_set_error() as necessary.
   *   - action_submit: An array of submission methods to call.
   *   as hinted, these are /methods/, which should exist on the given class
   *   (or one of its ancestors).
   */
  protected function getActions(&$form_state) {
    $coursepack = islandora_object_load($_GET['pid']);
    $groups = array();
    $group['wrapper'] = array(
      '#type' => 'fieldset',
      '#title' => t('Add selected objects to @label', array('@label' => $coursepack->label)),
    );

    $group['wrapper']['coursepack_identifier'] = array(
      '#type' => 'hidden',
      '#value' => $_GET['pid'],
    );

    $group['wrapper']['coursepack_add_selected_submit'] = array(
      '#type' => 'submit',
      '#value' => t('Add selected results'),
      '#name' => 'coursepack_add_selected',
      '#weight' => 5,
      '#action_submit' => 'coursepackAddSelected',
      '#action_validate' => array(
        'hasObjectsSelected',
      ),
    );
    $group['wrapper']['coursepack_add_all_submit'] = array(
      '#type' => 'submit',
      '#value' => t('Add all results'),
      '#name' => 'coursepack_add_all',
      '#weight' => 5,
      '#action_submit' => 'coursepackAddAll',
    );

    $groups['islandora_build_collection'] = array(
      'fieldset' => $group,
    );

    return $groups;
  }

  /**
   * One of our '#action_validate' callbacks.
   *
   * Checks that some objects are selected.
   */
  protected function hasObjectsSelected($form, &$form_state) {
    $pids = $this->getSelected($form_state);
    if (empty($pids)) {
      form_set_error('islandora_bookmark_table', t('You must select at least one object.'));
    }
  }

  /**
   * One of our '#action_validate' callbacks.
   *
   * Checks that some objects are selected.
   */
  protected function hasParentSelected($form, &$form_state) {
    if ($form_state['values']['migration_action'] == 'existing') {
      $object = NULL;
      if (isset($form_state['values']['new_collection_name'])) {
        $object = islandora_object_load($form_state['values']['new_collection_name']);
      }
      if (!$object) {
        form_set_error('new_collection_name', t('You must select parent collection.'));
      }
    }
  }

  /**
   * One of our '#action_submit' callbacks.
   *
   * Migrate all PIDs the search yielded.
   *
   * @param array $form
   *   The array representing a Drupal form.
   * @param array $form_state
   *   The array of form state.
   */
  protected function coursepackAddAll(&$form, &$form_state) {
    $pids = $this->getAll($form_state);
    $this->coursepackAdd($pids, $form_state);
  }

  /**
   * One of our '#action_submit' callbacks.
   *
   * Migrate the selected PIDs the search yielded.
   *
   * @param array $form
   *   The array representing a Drupal form.
   * @param array $form_state
   *   The array of form state.
   */
  protected function coursepackAddSelected(&$form, &$form_state) {
    $pids = $this->getSelected($form_state);
    $this->coursepackAdd($pids, $form_state);
  }

  /**
   * Actually handle the additon of objects.
   *
   * @param array $pids
   *   Multiple PIDs, as strings.
   * @param array $form_state
   *   The array of form state.
   */
  protected function coursepackAdd($pids, &$form_state) {
    $objects = array();
    foreach($pids as $pid) {
      $objects[] = islandora_object_load($pid);
    }
    $objects_to_add = array();
    $identifier = $form_state['values']['coursepack_identifier'];
    $current_components = islandora_compound_object_get_parts($identifier);
    foreach ($objects as $object) {
      if (!in_array($object->id, $current_components)) {
        $objects_to_add[] = $object;
      }
    }
    if ($objects_to_add) {
      $pids = $form_state['values']['coursepack_identifier'];
      module_load_include('inc', 'islandora_compound_object', 'includes/manage.form');
      islandora_compound_object_add_parent($objects, $identifier);
    }
    drupal_goto("islandora/object/$identifier");
  }

  /**
   * Go over any '#action_validate' functions on the clicked button.
   */
  public function formValidate(&$form, &$form_state) {
    $clicked_button = $form_state['clicked_button'];
    if (isset($clicked_button['#action_validate'])) {
      foreach ((array) $clicked_button['#action_validate'] as $validator) {
        call_user_func(array($this, $validator), $form, $form_state);
      }
    }
  }

  /**
   * Submit handler for the islandora_bookmark_results form.
   *
   * Go over any '#action_submit' functions on the clicked button.
   *
   * @param array $form
   *   The Drupal form definition.
   * @param array $form_state
   *   The Drupal form state.
   */
  public function formSubmit(&$form, &$form_state) {
    $queries = drupal_get_query_parameters();

    $form_state['redirect'] = array(request_path(), array('query' => $queries));

    $clicked_button = $form_state['clicked_button'];
    if (isset($clicked_button['#action_submit'])) {
      $params = array(&$form, &$form_state);
      foreach ((array) $clicked_button['#action_submit'] as $submit_function) {
        call_user_func_array(array($this, $submit_function), $params);
      }
    }
  }

  /**
   * Submission helper; gets the PIDs all results.
   */
  protected function getAll(&$form_state) {
    $pids = array();

    // Helper anonymous function... Just get the PID.
    $get_pid = function($result) {
          return $result['PID'];
        };

    $qp = $this->islandoraSolrQueryProcessor;
    $old_params = $qp->solrParams;
    $qp->solrParams['fl'] = 'PID';
    $qp->solrStart = 0;
    $qp->solrLimit = 10000;

    // Handle the first set separately, so we can get the total number for our
    // loop.
    $qp->executeQuery();
    $solr_results = $qp->islandoraSolrResult;
    $result_count = $solr_results['response']['numFound'];
    $object_results = $solr_results['response']['objects'];
    $pids = array_map($get_pid, $object_results);

    for ($i = $qp->solrLimit; $i < $result_count; $i += $qp->solrLimit) {
      $qp->solrStart = $i;
      $qp->executeQuery();
      $solr_results = $qp->islandoraSolrResult;
      $object_results = $solr_results['response']['objects'];

      $pids = array_merge(
          $pids, array_map($get_pid, $object_results)
      );
    }
    $qp->solrParams = $old_params;

    return $pids;
  }

  /**
   * Submission helper; gets the PIDs of the selected results.
   */
  protected function getSelected(&$form_state) {
    $selected = array_filter($form_state['values']['islandora_solr_select_table']);
    $functional = array_filter($selected, 'islandora_object_load');
    $broken = array_diff($selected, $functional);
    if (!empty($broken)) {
      watchdog('islandora_bookmark', 'Broken PIDs encountered: @pids', array(
        '@pids' => implode(', ', $broken),
          ), WATCHDOG_WARNING);
    }
    foreach ($selected as $key => $value) {
      $object = islandora_object_load($key);
      if ($object) {
        $objects[] = $object;
      }
    }
    return $objects;
  }

}

/**
 * Form building function.
 *
 * @param array $form
 *   The Drupal form definition.
 * @param array $form_state
 *   The Drupal form state.
 * @param IslandoraSolrResultsBookmark $results_class
 *   An object on which we can call buildForm() to get our form structure.
 * @param array $object_results
 *   The object results from the Islandora Solr processor created in the
 *   printResults function.
 *
 * @return array
 *   An array containing form structure.
 */
function islandora_collection_builder_results_form($form, &$form_state, $results_class, $object_results) {
  return $results_class->formGet($form, $form_state, $object_results);
}

/**
 * Form validation handler.
 */
function islandora_collection_builder_results_form_validate(array &$form, array &$form_state) {
  $form_state['build_info']['args'][0]->formValidate($form, $form_state);
}

/**
 * Submit handler for the islandora_bookmark_results form.
 *
 * @param array $form
 *   The Drupal form definition.
 * @param array $form_state
 *   The Drupal form state.
 */
function islandora_collection_builder_results_form_submit(array &$form, array &$form_state) {
  $form_state['build_info']['args'][0]->formSubmit($form, $form_state);
}
