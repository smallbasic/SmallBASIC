package net.sourceforge.smallbasic;

import android.location.Location;
import android.os.Bundle;

public class LocationListener implements android.location.LocationListener {
  private Location _location;

  public LocationListener() {
    setLocation(null);
  }

  public Location getLocation() {
    return _location;
  }

  @Override
  public void onLocationChanged(Location location) {
    setLocation(location);
  }

  @Override
  public void onProviderDisabled(String provider) {
  }

  @Override
  public void onProviderEnabled(String provider) {
  }

  @Override
  public void onStatusChanged(String provider, int status, Bundle extras) {
  }

  public void setLocation(Location _location) {
    this._location = _location;
  }
}
