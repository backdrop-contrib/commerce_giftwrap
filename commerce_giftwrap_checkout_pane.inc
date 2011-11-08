<?php
/**
 * @file
 * Contains all code for the gift wrap pane. Called from commerce_giftwrap_commerce_checkout_pane_info()
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
  return $form;
}

/**
 * Implements base_checkout_form()
 */
function commerce_giftwrap_pane_checkout_form($form, $form_state, $checkout_pane, $order) {
  
  //Set $default_value
  if(commerce_giftwrap_giftwrap_line_item_exists($order)){
    $default_value = 1;
  } else {
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
    
  $checkout_form['commerce_giftwrap_additional_info'] = array(
    '#markup' => '<div class="giftwrap-additonal-info">
                    <a href="#">What\'s included?</a>
                    <p>' . variable_get('commerce_giftwrap_additional_info', '') . '</p>
                  </div>',
  );
    
  return $checkout_form;
}

/**
 * Implements base_checkout_form_submit()
 */
function commerce_giftwrap_pane_checkout_form_submit($form, &$form_state, $checkout_pane, $order) {
  
  $giftwrap_line_item_exists = commerce_giftwrap_giftwrap_line_item_exists($order);
  
  $giftwrap_decision = $form_state['values']['commerce_giftwrap']['commerce_giftwrap_decision'];
  
  if(!$giftwrap_line_item_exists && $giftwrap_decision){
    
    // Set the currency code
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
    //$line_item_wrapper->title = 'Gift Wrapping';
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
    // Save the order
    commerce_order_save($order);
    
  } elseif (!$giftwrap_decision) {
    // If the user selects 'No' check if giftwrapping has been added, if so delete it.
    commerce_giftwrap_delete_shipping_line_items($order);
  }
  
}