<?php

function agile_coursepack_clone_form($form, $form_state, $pid) {
  $object = islandora_object_load($pid);
  $form['intro'] = array(
    '#markup' => t("Clone @label", array('@label' => $object->label)),
  );
  $form['new_name'] = array(
    '#type' => 'textfield',
    '#title' => t('Title'),
    '#description' => t('New name for cloned coursepack'),
    '#default_value' => t("Copy of @label", array('@label' => $object->label)),
  );
  $form['original'] = array(
    '#type' => 'hidden',
    '#value' => $pid,
  );
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t("Clone coursepack"),
  );
  return $form;
}

function agile_coursepack_clone_form_submit($form, $form_state) {
  module_load_include('inc', 'islandora_compound_object', 'includes/manage.form');
  $tuque = new IslandoraTuque();
  $namespace = variable_get('agile_coursepack_object_namespace', 'islandora');
  $new_pid = $tuque->api->m->getNextPid($namespace);
  $old_object = islandora_object_load($form_state['values']['original']);
  $new_object = islandora_copy_object($old_object);
  $new_object->label = $form_state['values']['new_name'];
  $new_object->id = $new_pid;
  $object = islandora_add_object($new_object);
  $child_keys = array_keys(islandora_compound_object_get_parts($old_object->id, TRUE));
  foreach($child_keys as $child) {
    $children[] =islandora_object_load($child);
  }
  islandora_compound_object_add_parent($children, $object->id);
  drupal_goto("islandora/object/{$object->id}");
}
