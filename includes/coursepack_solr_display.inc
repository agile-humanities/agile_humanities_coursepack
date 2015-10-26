<?php

/**
 * @file
 * Contains methods to search solr and display results. Depends on
 * Apache_Solr_Php client.
 * Default sear  display for Coursepack Sites.
 */

/**
 * Extention of IslandoraSolrResults to create an alternative display type.
 */
class IslandoraSolrResultsCoursepackDisplay extends IslandoraSolrResultsBookmark {

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
      'markup' => t("Digital Asset"),
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


