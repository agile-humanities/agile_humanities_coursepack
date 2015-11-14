<?php

/**
 * Splash page.
 *
 * @param array $form
 *   The drupal form.
 * @param array $form_state
 *   The drupal form state.
 *
 * @return array
 *   The drupal form definition.
 */
function agile_coursepack_splash(array $form, array &$form_state) {
  if (isset($form_state['triggering_element'])) {
    if ($form_state['triggering_element']['#id'] == 'agile-coursepack-add') {
      drupal_goto('agile_coursepack/add_item');
    }
    if ($form_state['triggering_element']['#id'] == 'agile-coursepack-browse') {
      drupal_goto('agile_coursepack/browse');
    }
    if ($form_state['triggering_element']['#id'] == 'agile-coursepack-create') {
      drupal_goto('agile_coursepack/add_coursepack');
    }
    
  }
  $module_path = drupal_get_path('module', 'agile_coursepack');
  drupal_add_css("$module_path/css/agile_coursepack_splash.css");
  $about_id = variable_get('agile_coursepack_about_node_id', FALSE);
  $about_node = node_load($about_id);
  if ($about_node) {
    $body = field_get_items('node', $about_node, 'body');
    $body_value = $body[0]['value'];
  }
  else {
    $body_value = t("<strong>@body</strong>", array('@body' => 'Welcome to the coursepack landing page.'));
  }

  $form['about_container'] = array(
    '#type' => 'container',
    '#attributes' => array(
      'id' => array('splash_button_about'),
    ),
  );
  $form['about_container']['about'] = array(
    '#attributes' => array('class' => array('agile-coursepack-about')),
    '#markup' => $body_value,
  );

  $buttons = agile_coursepack_buttons();
  $form = array_merge($form, $buttons);

  return $form;
}

function agile_coursepack_buttons() {
  $form = array();
  $form['buttons'] = array(
    '#type' => 'container',
    '#attributes' => array(
      'id' => array('splash_button_id'),
    ),
  );

  $form['buttons']['browse'] = array(
    '#type' => 'button',
    '#value' => t("Browse"),
    '#attributes' => array('class' => array('agile-coursepack-splash-button')),
    '#id' => 'agile-coursepack-browse',
  );

  $form['buttons']['create'] = array(
    '#type' => 'button',
    '#value' => t("Create new coursepack"),
    '#attributes' => array('class' => array('agile-coursepack-splash-button')),
    '#id' => 'agile-coursepack-create',
  );

  $form['buttons']['add'] = array(
    '#type' => 'button',
    '#value' => t("Add new item"),
    '#attributes' => array('class' => array('agile-coursepack-splash-button')),
    '#id' => 'agile-coursepack-add',
  );
  return $form;
}
