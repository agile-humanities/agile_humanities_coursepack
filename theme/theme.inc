<?php

/**
 * @file
 * This file contains all theme and preprocess functions.
 */

/**
 * Implements hook_preprocess_theme().
 */
function agile_coursepack_preprocess_agile_coursepack(array &$variables) {
  module_load_include('inc', 'agile_coursepack', 'includes/utilities');
  module_load_include('inc', 'agile_coursepack', 'includes/coursepack_datastream');
  $object = $variables['islandora_object'];
  $cp = $object['CPDATA']->content;
  $cpdata = new CPData($cp);
  $children = islandora_compound_object_get_parts($object->id, TRUE);
  $links = array();
  foreach ($children as $pid => $values) {
    $links[] = l($values['title'], "islandora/object/$pid");
  }
  $list_variables = array();
  if ($links) {
    $list_variables = array(
      'items' => $links,
      'title' => t('Reading list for @coursepack', array('@coursepack' => $object->label)),
      'type' => 'ul',
      'attributes' => array('class' => 'coursepack_members'),
    );
  }
  $variables['links'] = NULL;
  if ($list_variables) {
    $variables['links'] = theme_item_list($list_variables);
  }
  $block = module_invoke('agile_coursepack', 'block_view', 'coursepack_solr');
  $variables['search_block'] = NULL;
  $variables['solr_link'] = NULL;
  $variables['metadata_link'] = NULL;
  $variables['manage_link'] = NULL;
  if (agile_coursepack_can_edit($object)) {
    $variables['search_block'] = render($block['content']);
    $variables['solr_link'] = l(t("Add items to coursepack"), "agile_coursepack/solr_redirect/{$object->id}");
    $variables['metadata_link'] = l(t("Edit course details and description"), "agile_coursepack/add_coursepack/{$object->id}");
    if ($links) {
      $variables['manage_link'] = l(t("Rearrange or delete items"), "islandora/object/{$object->id}/manage/compound");
    }
  }

  $variables['consolidate_link'] = l(t("Download coursepack material"), "agile_coursepack/consolidate/{$object->id}");
  if (user_access('create new coursepacks')) {
    $variables['clone_link'] = l(t("Duplicate and modify this coursepack"), "agile_coursepack/clone/{$object->id}");
  }
  $variables['document'] = theme('islandora_document', array('islandora_object' => $object));
  $variables['attribution'] = $cpdata->getElement("cp:attribution");
  $contact_email = variable_get('agile_coursepack_admin_email', FALSE);
  if ($contact_email) {
    $variables['contact'] = l('Contact Administrator if any content in this coursepack violates copyright.', "mailto:$contact_email", array('absolute' => TRUE));
  }
}
