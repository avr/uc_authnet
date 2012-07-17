<?php
// $Id$

/**
 * @file
 * Includes page callbacks for administrative recurring fee operation pages.
 */
/**
 * Callback for payment gateway settings.
 */
function _uc_authnet_settings_form() {
  $login_data = _uc_authnet_login_data();

  $form['api_id_key'] = array(
    '#type' => 'fieldset',
    '#title' => t('API Login ID and Transaction Key'),
    '#description' => t('This information is required for Ubercart to interact with your payment gateway account. It is different from your login ID and password and may be found through your account settings page. Do not change the gateway URLs unless you are using this module with an Authorize.net-compatible gateway that requires different URLs.'),
  );
  $form['api_id_key']['uc_authnet_api_login_id'] = array(
    '#type' => 'textfield',
    '#title' => t('API Login ID'),
    '#default_value' => variable_get('uc_authnet_api_login_id', ''),
  );
  $form['api_id_key']['uc_authnet_api_transaction_key'] = array(
    '#type' => 'textfield',
    '#title' => t('Transaction Key'),
    '#default_value' => variable_get('uc_authnet_api_transaction_key', ''),
  );
  $form['api_id_key']['uc_authnet_api_test_gateway_url'] = array(
    '#type' => 'textfield',
    '#title' => t('Authorize.net Test Gateway URL'),
    '#default_value' => variable_get('uc_authnet_api_test_gateway_url', UC_AUTHORIZENET_TEST_GATEWAY_URL),
  );
  $form['api_id_key']['uc_authnet_api_live_gateway_url'] = array(
    '#type' => 'textfield',
    '#title' => t('Authorize.net Live Gateway URL'),
    '#default_value' => variable_get('uc_authnet_api_live_gateway_url', UC_AUTHORIZENET_LIVE_GATEWAY_URL),
  );

  $form['aim_settings'] = array(
    '#type' => 'fieldset',
    '#title' => t('Transaction settings'),
    '#description' => t('These settings pertain to the Authorize.Net AIM payment method for card not present & eCheck transactions.'),
  );
  $form['aim_settings']['uc_authnet_aim_txn_mode'] = array(
    '#type' => 'radios',
    '#title' => t('Transaction mode'),
    '#description' => t('Only specify a developer test account if you login to your account through https://test.authorize.net.<br />Adjust to live transactions when you are ready to start processing real payments.'),
    '#options' => array(
      'live' => t('Live transactions in a live account'),
      'live_test' => t('Test transactions in a live account'),
      'developer_test' => t('Developer test account transactions'),
    ),
    '#default_value' => variable_get('uc_authnet_aim_txn_mode', 'live_test'),
  );
  
  // Allow admin to set duplicate window
  $form['aim_settings']['uc_authnet_duplicate_window'] = array(
    '#type' => 'select',
    '#title' => t('Duplicate window'),
    '#description' => t('Blocks submission of duplicate transactions within the specified window.  Defaults to 120 seconds.'),
    '#default_value' => variable_get('uc_authnet_duplicate_window', 120),
    '#options' => drupal_map_assoc(array(0, 15, 30, 45, 60, 75, 90, 105, 120)),
  );
  $form['aim_settings']['uc_authnet_aim_email_customer'] = array(
    '#type' => 'checkbox',
    '#title' => t('Tell Authorize.net to e-mail the customer a receipt based on your account settings.'),
    '#default_value' => variable_get('uc_authnet_aim_email_customer', FALSE),
  );
  $form['aim_settings']['uc_authnet_response_debug'] = array(
    '#type' => 'checkbox',
    '#title' => t('Log full API response messages from Authorize.net for debugging.'),
    '#default_value' => variable_get('uc_authnet_response_debug', FALSE),
  );

  if (module_exists('uc_echeck')) {
    $form['echeck_settings'] = array(
      '#type' => 'fieldset',
      '#title' => t('eCheck settings'),
      '#description' => t('These settings pertain to the Authorize.Net eCheck service. This is an add on service that is not included by default with your Authorize.net gateway fees.')
    );
    $form['echeck_settings']['uc_authnet_echeck'] = array(
      '#type' => 'checkbox',
      '#title' => t('Enable the eCheck.net service as a payment method.'),
      '#default_value' => variable_get('uc_authnet_echeck', FALSE),
    );
  }

  if (module_exists('uc_cim')){
    $form['cim_settings'] = array(
      '#type' => 'fieldset',
      '#title' => t('CIM settings'),
      '#description' => t('These settings pertain to the Authorize.Net Customer Information Management service.')
    );
    $form['cim_settings']['uc_authnet_cim_profile'] = array(
      '#type' => 'checkbox',
      '#title' => t('Always create a CIM profile for securely storing payment (CC or eCheck) info for later use.'),
      '#default_value' => variable_get('uc_authnet_cim_profile', FALSE),
    );
  }

  return $form;
}
/**
 * Implementation of hook_form_alter().
 */
function uc_authnet_form_alter(&$form, $form_state, $form_id) {
  if ($form_id == 'uc_payment_gateways_form') {
    $form['#submit'][] = '_uc_authnet_payment_gateway_settings_submit';
  }
}

/**
 * Submit handler for payment gateway settings form to encrypt fields.
 */
function _uc_authnet_payment_gateway_settings_submit($form, &$form_state) {
  // If CC & eCheck encryption have been configured properly.
  if ($key = uc_credit_encryption_key() || $echeck_key = uc_echeck_encryption_key()) {
    // Setup our encryption object.
    $crypt = new uc_encryption_class;

    // Encrypt the Login ID, Transaction key, and MD5 Hash.
    if (!empty($form_state['values']['uc_authnet_md5_hash'])) {
      variable_set('uc_authnet_md5_hash', $crypt->encrypt($key, $form_state['values']['uc_authnet_md5_hash']));
    }

    // Store any errors.
    uc_store_encryption_errors($crypt, 'uc_authnet');
  }
}

/*
// Displays a form to update a subscriptions's CC info.
function uc_authorizenet_arb_admin_update_form($form_state, $rfid) {
  $order = new stdClass();
  $form = array();

  $fee = uc_recurring_fee_load('user', $rfid);

  $form['rfid'] = array(
    '#type' => 'value',
    '#value' => $rfid,
  );
  $form['description'] = array(
    '#value' => '<div>'. t('Subscription ID: @subscription_id', array('@subscription_id' => $fee['data'])) .'</div>',
  );

  $form['cc_data'] = array(
    '#type' => 'fieldset',
    '#title' => t('Credit card details'),
    '#theme' => 'uc_payment_method_credit_form',
    '#tree' => TRUE,
  );
  $form['cc_data'] += uc_payment_method_credit_form(array(), $order);
  unset($form['cc_data']['cc_policy']);

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Update'),
    '#suffix' => l(t('Cancel'), 'admin/store/orders/recurring'),
  );

  return $form;
}

function uc_authorizenet_arb_admin_update_form_submit($form, &$form_state) {
  $fee = uc_recurring_fee_load('user', $form_state['values']['rfid']);

  $updates = array(
    'payment' => array(
      'creditCard' => array(
        'cardNumber' => $form_state['values']['cc_data']['cc_number'],
        'expirationDate' => $form_state['values']['cc_data']['cc_exp_year'] .'-'. $form_state['values']['cc_data']['cc_exp_month'],
      ),
    ),
  );

  $result = uc_authorizenet_arb_update($fee['data'], $updates, $fee['order_id']);

  // If the update was successful...
  if ($result) {
    drupal_set_message(t('Subscription data updated at Authorize.Net.'));
  }
  else {
    drupal_set_message(t('Subscription update failed. See order admin comments for more details.'), 'error');
  }

  $form_state['redirect'] = 'admin/store/orders/recurring';
}

// Displays a confirm form for cancelling a subscription.
function uc_authorizenet_arb_admin_cancel_form($form_state, $rfid) {
  $form['rfid'] = array(
    '#type' => 'value',
    '#value' => $rfid,
  );

  return confirm_form($form, t('Are you sure you wish to cancel this subscription?'), 'admin/store/orders/recurring', NULL, t('Confirm'), t('Cancel'));
}

function uc_authorizenet_arb_admin_cancel_form_submit($form, &$form_state) {
  $fee = uc_recurring_fee_load('user', $form_state['values']['rfid']);

  $result = uc_authorizenet_arb_cancel($fee['data'], $fee['order_id'], $fee);

  // If the cancellation was successful...
  if ($result) {
    drupal_set_message(t('Subscription cancelled through Authorize.Net.'));

    // Set the fee's recurring charges to 0.
    uc_recurring_fee_cancel($fee['rfid']);
  }
  else {
    drupal_set_message(t('Subscription cancellation failed. See order admin comments for more details.'), 'error');
  }

  $form_state['redirect'] = 'admin/store/orders/recurring';
}
*/