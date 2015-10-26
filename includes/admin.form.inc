<?php

function agile_coursepack_admin_settings_form(array $form, array &$form_state) {
  $about_node = variable_get('agile_coursepack_about_node_id', '');
  $namespace = variable_get('agile_coursepack_object_namespace', 'islandora');
  if ($about_node) {
    $form['about'] = array(
      '#type' => 'markup',
      '#markup' => l(t('Go to node'), "/node/$about_node/edit"),
    );
  }
  $form['agile_coursepack_about_node_id'] = array(
    '#type' => 'textfield',
    '#title' => t('Node ID of About page text'),
    '#description' => t("The Drupal node containing the text for the Coursepack splash page"),
    '#default_value' => $about_node,
    '#size' => 5,
  );
  $form['agile_coursepack_object_namespace'] = array(
    '#type' => 'textfield',
    '#title' => t('Namespace for created objects'),
    '#description' => t("Namespace for PIDs of objects created with this module"),
    '#default_value' => $namespace,
    '#size' => 5,
  );
  return system_settings_form($form);
}
