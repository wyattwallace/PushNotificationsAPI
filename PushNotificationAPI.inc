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

  abstract public function getPushData( $payload );

  public function push( IPushableObserved $sender )
  {
    $payload = $sender->getPayload();

    if ( true === $this->isNotificationRatelimited( $payload->toMemberId, $payload->fromMemberId ) )
    {
      return true;
    }

    if ( false === $this->userSettingsAllowsNotification( ) )
    {
      return true;
    }

    $data = $this->getPushData( $payload );
    
    $this->sendNotification( $data );
  }

  protected function sendNotification( $data )
  {

    // send into queue fQueue or mybPublish ?

  }

  /** 
   * check to see if the notification is ratelimited
   *
   * Default Rules:
   * User A can only send X push notifications 
   * to User B in N minutes (X=2, N=10)
   *
   * User B can only receive X total push notifications 
   * per N minutes  (X=10, N=60)
   * 
   * User B can only receive A total push notifications 
   * per N hours  (A=50, N=24)
   *
   * if the $from user is 0, ignore limiting
   *
   * @param int $to recipient of notification 
   * @param int $from sender of the notification
   * @return bool
   */
  protected function isNotificationRatelimited( $to, $from )
  {
    // check rate limit factoring in Device Type self::NOTIFICATION_SETTING_TYPE
    
  }

  protected function userSettingsAllowsNotification( )
  {
    // check notifications settings self::NOTIFICATION_SETTING_TYPE

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
      if ( ! $obs->push( $this ) ) 
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

  public function __construct( $notificationType )
  {
    // get name of decoratable class
    $className = $this->getDecoratableClassName( $notificationType );

    // if class exists check

    $this->pushNotificationObject = new $className();
  }

  public function getNotificationData( $payload )
  {
    return $this->pushNotificationObject->getNotificationData( $payload );
  }

  protected function getDecoratableClassName( $notificationType )
  {
    // combine to form class name - PushNotificationNewProfileView
    return 'PushNotification' . ucfirst( $notificationType );
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
  public function getNotificationData( array $payload );
}

class PushNotificationNewProfileView implements IPushNotificationDecoratable
{

  /**
   *
   * @param array $data
   * @access public
   * @abstract
   */
  public function getNotificationData( array $payload )
  {
    // look for expected data, if not found the add 
    $data = array();

    if ( isset( $payload['firstName'] ) )
    {
      // first name set, so use it
      $data['firstName'] = $payload['firstName'];
    }
    elseif ( isset( $payload['fromMemberId'] ) )
    {
      // no first name, but we have the memberId, so look up firstName
      $userData = new UserData( $payload['fromMemberId'] );
      $userDataArray = $userData->getUserData( array( 'firstName' ) );
      $data['firstName'] = $userDataArray[0]->firstName;
    }
    else
    {
      // no firstName and no memberId so throw exception
    }

    return $data;
  }
}



class PushableDeviceIphone extends AbstractPushableObject
{
  const NOTIFICATION_SETTING_TYPE = 'iphone';
  const QUEUE_NAME = 'iphone_queue_name';
  public $er =  Array();

  public function getPushData( $payload )
  {
    $pushNotificationDecorator = new PushNotificationDecorator( $payload['notificationType'] );
    $data = $pushNotificationDecorator->processData( $payload );

    // do magic here

    return $data;
  }

}

class PushableDeviceIpad extends AbstractPushableObject
{
  const NOTIFICATION_SETTING_TYPE = 'ipad';
  const QUEUE_NAME = 'ipad_queue_name';
  public $er =  Array();

  public function getPushData( $payload )
  {
    $pushNotificationDecorator = new PushNotificationDecorator( $payload['notificationType'] );
    $data = $pushNotificationDecorator->processData( $payload );

    // do magic here

    return $data;
  }

}

class PushableDeviceAndroid extends AbstractPushableObject
{
  const NOTIFICATION_SETTING_TYPE = 'android';
  const QUEUE_NAME = 'android_queue_name';
  public $er =  Array();

  public function getPushData( $payload )
  {
    $pushNotificationDecorator = new PushNotificationDecorator( $payload['notificationType'] );
    $data = $pushNotificationDecorator->processData( $payload );

    // do magic here

    return $data;
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
