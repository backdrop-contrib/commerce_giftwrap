<?php
/**
 * @file
 * Contains all code for the gift wrap pane. Called from
 *  commerce_giftwrap_commerce_checkout_pane_info()
 */

/**
 * Implements base_settings_form()
 */
function commerce_giftwrap_pane_settings_form($checkout_pane) {
  $form = array();
  $form['commerce_giftwrap_price'] = array(
    '#type' => 'textfield',
    '#title' => t('Gift Wrapping Price'),
    '#size' => 10,
    '#default_value' => variable_get('commerce_giftwrap_price', '2.00'),
  );
  $form['commerce_giftwrap_additional_info'] = array(
    '#type' => 'textarea',
    '#title' => t('Gift Wrapping Information'),
    '#description' => t('If you add a description a link to some more information about the giftwrapping service will be
                          displayed below the Yes/No form field.'),
    '#default_value' => variable_get('commerce_giftwrap_additional_info', ''),
  );
  $form['commerce_giftwrap_show_message'] = array(
    '#type' => 'radios',
    '#title' => t('Allow the user to add a message?'),
    '#options' => array(
      1 => t('Yes'),
      0 => t('No'),
    ),
    '#default_value' => variable_get('commerce_giftwrap_show_message', '1'),
  );
  $form['commerce_giftwrap_limit_message'] = array(
    '#type' => 'textfield',
    '#title' => t("Limit the length of the user's message"),
    '#size' => 5,
    '#default_value' => variable_get('commerce_giftwrap_limit_message', '60'),
  );
  return $form;
}
/**
 * Implements base_checkout_form()
 */
function commerce_giftwrap_pane_checkout_form($form, &$form_state, $checkout_pane, $order) {
  // Set $default_value
  if (commerce_giftwrap_line_item_exists($order)) {
    $default_value = 1;
  }
  else {
    $default_value = 0;
  }
  $checkout_form['commerce_giftwrap_decision'] = array(
    '#type' => 'select',
    '#title' => t('Would you like your order gift wrapped?'),
    '#options' => array(
      0 => t('No'),
      1 => t('Yes'),
    ),
    '#default_value' => $default_value,
  );
  if (variable_get('commerce_giftwrap_additional_info', '') != '') {
    $checkout_form['commerce_giftwrap_additional_info'] = array(
      '#markup' => '<div class="giftwrap-additonal-info"><p>'
      . variable_get('commerce_giftwrap_additional_info', '') .
      '</p></div>',
    );
  }
  if (variable_get('commerce_giftwrap_show_message', '1')) {
    $checkout_form['commerce_giftwrap_message'] = array(
      '#type' => 'textarea',
      '#title' => t('Message'),
      '#description' => t('Add a personal message to be included with your gift. Your message must be no longer than @char characters.',
                            array('@char' => variable_get('commerce_giftwrap_limit_message', '60'))),
      '#maxlength' => variable_get('commerce_giftwrap_limit_message', '60'),
      '#states' => array(
        'visible' => array(
          ':input[name="commerce_giftwrap[commerce_giftwrap_decision]"]' => array('value' => '1'),
        ),
      ),
    );
  }
  return $checkout_form;
}
/**
 * Implements base_checkout_form_submit()
 */
function commerce_giftwrap_pane_checkout_form_submit($form, &$form_state, $checkout_pane, $order) {
  $giftwrap_line_item_exists = commerce_giftwrap_line_item_exists($order);
  $giftwrap_decision = $form_state['values']['commerce_giftwrap']['commerce_giftwrap_decision'];
  if (!$giftwrap_line_item_exists && $giftwrap_decision) {
    // Set the currency code.
    $default_currency_code = commerce_default_currency();
    if ($balance = commerce_payment_order_balance($order)) {
      $default_currency_code = $balance['currency_code'];
    }
    // Create the new line item.
    $line_item = commerce_line_item_new('giftwrap', $order->order_id);
    // Wrap the line item and order to simplify manipulating their field data.
    $order_wrapper = entity_metadata_wrapper('commerce_order', $order);
    $line_item_wrapper = entity_metadata_wrapper('commerce_line_item', $line_item);
    // Populate the $line_item_wrapper...
    $line_item_wrapper->line_item_label = 'Gift Wrapping';
    $line_item_wrapper->commerce_giftwrap_message = $form_state['values']['commerce_giftwrap']['commerce_giftwrap_message'];
    $line_item_wrapper->quantity = 1;
    $line_item_wrapper->commerce_unit_price->amount = variable_get('commerce_giftwrap_price', '200');
    $line_item_wrapper->commerce_unit_price->currency_code = $default_currency_code;
    // Set the price component of the unit price.
    $line_item_wrapper->commerce_unit_price->data = commerce_price_component_add(
      $line_item_wrapper->commerce_unit_price->value(),
      'giftwrap',
      $line_item_wrapper->commerce_unit_price->value(),
      TRUE,
      FALSE
    );
    // Save the incoming line item now so we get its ID.
    commerce_line_item_save($line_item);
    // Add it to the order's line item reference value.
    $order_wrapper->commerce_line_items[] = $line_item;
    // Save the order.
    commerce_order_save($order);
  }
  elseif (!$giftwrap_decision) {
    // If the user selects 'No' check if giftwrapping has been added, if so
    // delete it.
    commerce_giftwrap_delete_giftwrap_line_items($order);
  }
}
/**
 * Implements base_review()
 */
function commerce_giftwrap_pane_review($form, &$form_state, $checkout_pane, $order) {
  $order_wrapper = entity_metadata_wrapper('commerce_order', $order);
  foreach ($order_wrapper->commerce_line_items as $line_item) {
    // If this line item is a giftwrap line item...
    if ($line_item->type->value() == 'giftwrap') {
      $message = $line_item->commerce_giftwrap_message->value();
      break;
    }
  }
  if (commerce_giftwrap_line_item_exists($order)) {
    $output = array(
      'giftwrap_decision' => array(
        '#type' => 'markup',
        '#markup' => '<h3>Would you like your order gift wrapped? <strong>Yes</strong></h3>',
      ),
      'title' => array(
        '#type' => 'markup',
        '#markup' => '<h3>Message:</h3>',
      ),
      'giftwrap_message' => array(
        '#type' => 'markup',
        '#markup' => '<p>' . check_plain($message) . '</p>',
      ),
    );
  }
  else {
    $output['giftwrap_decision'] = array(
      '#type' => 'markup',
      '#markup' => '<h3>Would you like your order gift wrapped? <strong>No</strong></h3>',
    );
  }
  return drupal_render($output);
}
