<?php


interface IPushableObserved
{
  /**
   * Method for adding observer objects 
   *
   * @param object $observer
   * @access public
   * @abstract
   */
  public function addObserver( IPushableObserver $observer );

  /**
   * This is the method that notifies the attached/added observer objects of the state change
   *
   * @param array $payload
   * @access public
   * @abstract
   */
  public function push( array $payload );
}

interface IPushableObserver
{
  /**
   * All Observer Objects must implement this method.  Observered Object will notify Observers
   * through this method.
   *
   * @param object $sender
   * @access public
   * @abstract
   */
  public function send( IPushableObserved $sender );
}

abstract class AbstractPushableObject implements IPushableObserver
{

  public function sendNotification( $queueName, array $data )
  {

    // send into queue fQueue or mybPublish ?
    fQueue::getInstance( )->enqueue( $queueName, $data );

  }

}

class PushableObjectsList implements IPushableObserved
{
  public $err = array();

  private $payload = array();
  private $observers = array();

  public function push( array $payload )
  {
    $returnVal = true;

    // validate $payload here
    $this->payload = $payload;

    foreach( $this->observers as $obs ) 
    {
      if ( ! $obs->send( $this ) ) 
      {
        $this->err = array_merge( $this->err, $obs->err );
        $returnVal = false;
      }   
    }

    return $returnVal;
  }

  public function addObserver( IPushableObserver $observer )
  {
    $this->observers[] = $observer;
    return true;
  }

  public function getPayload()
  {
    return $this->payload;
  }
}

class PushNotificationDecorator
{
  $pushNotificationObject;

  public function __construct( $deviceType, $payload )
  {
    // get name of decoratable class
    $className = $this->getDecoratableClassName( $deviceType, $payload['routingKey'] );

    // if class exists check

    $this->pushNotificationObject = new $className();
  }

  public function processData( $payload )
  {
    return $this->pushNotificationObject->processData( $payload );
  }

  // we need to map the routing key to Push Notification type
  protected function getDecoratableClassName( $deviceType, $routingKey )
  {
    // string munge here

    // combine to form class name - PushNotificationNewProfileViewAndroid
    $string = 'PushNotification' . $routingKey . $deviceType;

    return $string;
  }

}

interface IPushNotificationDecoratable
{
  /**
   *
   * @param array $data
   * @access public
   * @abstract
   */
  public function processData( array $data );
}

class PushNotificationNewProfileViewAndroid implements IPushNotificationDecoratable
{

  /**
   *
   * @param array $data
   * @access public
   * @abstract
   */
  public function processData( array $data )
  {
    $returnArr = array();

    return $returnArr;
  }
}

/**
 * FACTOR IN EXISTING PushNotificationPayload CLASS
 */
abstract class PushNotificationPayload implements IPushNotificationDecoratable
{
  /**
   * 1. Remove second parameter frpm constructor signature
   *
   * 2. Rename getPayload() to decoratable method, processData() or whatever we come up with,
   *     and pass data/payload into it.  
   *
   * 3. Update signature of getAlert() and getExtendedData() to accept data/payload
   */
}


class PushNotificationNewProfileViewIOS extends PushNotificationPayload
{
  const MESSAGE = '%s just viewed your profile';

  /**
   * AMQP path will pass along minimum amount of data, we will pad payload
   * here with details associated with notification
   * 
   * @param array $data
   * @access public
   * @abstract
   */
  public function processData( array $data )
  {
    // look for expected data, if not found the add 
    $payload = array();
    
    if ( isset( $data['firstName'] ) )
    {
      // first name set, so use it
      $payload['firstName'] = $data['firstName'];
    }
    elseif ( isset( $data['fromMemberId'] ) )
    { 
      // no first name, but we have the memberId, so look up firstName
      $userData = new UserData( $data->fromMemberId );
      $userDataArray = $userData->getUserData( array( 'firstName' ) );
      $payload['firstName'] = $userDataArray[0]->firstName;
    }
    else
    {
      // no firstName and no memberId so throw exception
    }

    // call parents method that formats data correctly for iOS
    return parent::processData( $payload );
  }
}

class PushableDeviceIphone extends AbstractPushableObject
{
  const TYPE = 'iOS';
  const QUEUE_NAME = 'iphone_queue_name';
  public $er =  Array();

  public function send( IPushableObserved $sender )
  {
    $payload = $sender->getPayload();

    $pushNotificationDecorator = new pushNotificationDecorator( self::TYPE, $payload );
    $data = $pushNotificationDecorator->processData( $payload );

    $this->sendNotification( self::QUEUE_NAME, $data );
  }

}

class PushableDeviceIpad extends AbstractPushableObject
{
  const TYPE = 'iOS';
  const QUEUE_NAME = 'ipad_queue_name';
  public $er =  Array();

  public function send( IPushableObserved $sender )
  {
    $payload = $sender->getPayload();

    $pushNotificationDecorator = new pushNotificationDecorator( self::TYPE, $payload );
    $data = $pushNotificationDecorator->processData( $payload );

    $this->sendNotification( self::QUEUE_NAME, $data );
  }

}

class PushableDeviceAndroid extends AbstractPushableObject
{
  const TYPE = 'android';
  const QUEUE_NAME = 'android_queue_name';
  public $er =  Array();

  public function send( IPushableObserved $sender )
  {
    $payload = $sender->getPayload();

    $pushNotificationDecorator = new pushNotificationDecorator( self::TYPE, $payload );
    $data = $pushNotificationDecorator->processData( $payload );

    $this->sendNotification( self::QUEUE_NAME, $data );
  }

}

/* THIS IS AN EXISTING CLASS */
class PushNotifications
{

  /**
   * ActiveMQ API callback end point
   */
  public function sendNotification( $a, $b, $c, $d )
  {
    // 
    if ( false === $this->isRemote() )
    {
      $this->callRemotely( ... );
    }
    else
    {
      // build $payload 
      $payload = array();
      $payload['memberId'] = $a;
      $payload['toMemberId'] = $b;
      $payload['className'] = $c;

      // formerly called _sendNotification()
      $this->enqueueNotification( $payload );
    }
   
  }

  /**
   * AMQP API callback end point
   */
  public function processNotification( $payload )
  {
    // perform any preprocessing, if any, before calling enqueueNotificaiton()
    $this->enqueueNotification( $payload );
  }

  /**
   * Both AMQP and ActiveMQ end points may be different methods in this class, but both
   * will flow to this method.
   *
   * ActiveMQ end point will have more data, but will not have routing key
   * AMQP will need some data filled in
   * 
   * Method will contain all of the logic currently held in _sendNotification()
   */
  protected function enqueueNotification( $payload )
  {

    // note: in mybPublish memberId is reserved, always present
    $memberId = $payload->memberId;

    // check rate limiting

    // check notifications settings


    // get list of devices for this member
    $userDeviceLinks = $this->getUserDeviceLinks( $memberId );

    // instantiate observable object
    $pushableObjectsList = new PushableObjectsList();

    // add each device as an observer, each will handle the send in their own way
    foreach( $userDeviceLinks as $deviceKey => $deviceData )
    {
      list( $deviceType, $deviceId ) = explode( ':', $deviceKey );

      $className = 'PushableDevice' . ucfirst( $deviceType );
      // if class exists 
      $pushableDevice = new $className();
      $pushableObjectsList->addObserver( $pushableDevice );    
    }

    if ( ! $pushableObjectsList->push( $payload ) )
    {
      // handle error condition using $pushableObjectsList->err array 
    }
  }


}


?>
