<?php
/**
 * @file
 * Contains all code for the gift wrap local tasks on the order page. Called from commerce_payment_ui_menu().
 */
function commerce_giftwrap_order_tab($order) {
  $output =  array(
    
    'title' => array(
      '#type' => 'markup',
      '#markup' => '<h3>Giftwrap Message</h3>',
    ),
    'giftwrap_message' => array(
      '#type' => 'markup',
      '#markup' => '<p>' . $order->data['commerce_giftwrap_message'] . '</p>',
    ),
  );
  return $output;
}