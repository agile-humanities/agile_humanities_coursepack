<?php
/**
 * @file
 * Solr functions for coursepacks.
 */

/**
 * Gets all current coursepacks.
 *
 * @return array
 *   All coursepacks
 */
function agile_coursepack_get_coursepacks() {
  $results = agile_coursepack_get_results();
  $output = array();
  foreach ($results as $result) {
    $pid = $result['PID'];
    $course_number = $result['solr_doc']['mods_identifier_isbn_s'];
    $course_title = $result['solr_doc']['mods_titleInfo_title_s'];
    $key = "$course_number ~ $course_title ($pid)";
    $output[$key] = $key;
  }
  echo drupal_json_encode($output);
}

/**
 * Get all coursepacks as result set.
 *
 * @return array
 *   array of out
 */
function agile_coursepack_get_results() {
  $qp = new IslandoraSolrQueryProcessor();
  $qp->buildQuery('RELS_EXT_hasModel_uri_mlt:"info:fedora/islandora:coursepackCModel"');
  $qp->solrParams['fl'] = "PID  mods_identifier_isbn_s mods_Instructors_personal_namePart_s mods_titleInfo_title_s";
  $qp->solrParams['sort'] = "mods_titleInfo_title_ss asc";
  $qp->solrParams['facet.mincount'] = 1;
  $qp->executeQuery(FALSE);
  return $qp->islandoraSolrResult['response']['objects'];
}
