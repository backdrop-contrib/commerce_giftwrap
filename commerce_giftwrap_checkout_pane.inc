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
  
  dpm($form_state);
    
  $checkout_form['commerce_giftwrap_decision'] = array(
    '#type' => 'select',
    '#title' => t('Would you like your order gift wrapped?'), 
    '#options' => array(
      0 => t('No'),
      1 => t('Yes'),
    ),
    '#default_value' => 0,
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
  
  //make sure gift wrap is only added once
  //set default
  $giftwrap_line_item_exists = FALSE;
  
  foreach($order->commerce_line_items['und'] as $item) {
    $line_item_id = $item['line_item_id'];
    $line_item = commerce_line_item_load($line_item_id);
    if($line_item->type == 'giftwrap'){
      $giftwrap_line_item_exists = TRUE;
      $giftwrap_line_item_id = $line_item_id;
    }
  }
  
  $giftwrap_decision = $form_state['values']['commerce_giftwrap']['commerce_giftwrap_decision'];
  
  if(!$giftwrap_line_item_exists && $giftwrap_decision){
    
    //set the currency code
    $default_currency_code = commerce_default_currency();
    if ($balance = commerce_payment_order_balance($order)) {
      $default_currency_code = $balance['currency_code'];
    }

    // Create the new line item.
    $line_item = commerce_line_item_new('giftwrap', $order->order_id);

    // Wrap the order to simplify manipulating its field data.
    $order_wrapper = entity_metadata_wrapper('commerce_order', $order);

    // Wrap the line item to simplify manipulating its field data.
    $line_item_wrapper = entity_metadata_wrapper('commerce_line_item', $line_item);

    // Other code to populate the $line_item_wrapper...
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

    commerce_order_save($order);
    
  } elseif (!$giftwrap_decision) {
    // Remove the line item from the line item reference field.
    $order = commerce_entity_reference_delete('commerce_order', $order, 'commerce_line_items', $giftwrap_line_item_id);
    // Delete the actual line item.
    commerce_line_item_delete($giftwrap_line_item_id);
    //save the order
    commerce_order_save($order);
  }
  
}

/**
 * Implements base_checkout_form_review()
 */
function commerce_giftwrap_pane_review($form, &$form_state, $checkout_pane, $order) {
  $giftwrap_decision = $form_state['values']['commerce_giftwrap']['commerce_giftwrap_decision'];
  
  if($giftwrap_decision) {
    print 'Gift Wrap? Yes';
  } else {
    print 'Gift Wrap? No';
  }
}