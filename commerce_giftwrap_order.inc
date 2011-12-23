<?php
/**
 * @file
 * Contains all code for the gift wrap local tasks on the order page. Called from commerce_payment_ui_menu().
 */
function commerce_giftwrap_order_tab($order) {
  $order_wrapper = entity_metadata_wrapper('commerce_order', $order);

  foreach ($order_wrapper->commerce_line_items as $line_item) {
    // If this line item is a giftwrap line item...
    if ($line_item->type->value() == 'giftwrap') {
      $message = $line_item->commerce_giftwrap_message->value();
    }
  }
  
  $output =  array(
    'title' => array(
      '#type' => 'markup',
      '#markup' => '<h3>Giftwrap Message</h3>',
    ),
    'giftwrap_message' => array(
      '#type' => 'markup',
      '#markup' => '<p>' . $message . '</p>',
    ),
  );
  return $output;
}