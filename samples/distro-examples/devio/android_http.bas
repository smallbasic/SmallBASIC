import android

'
' request usage: endpoint [, data, apiKey]
' data, apikey: uses POST to send data, apiKey applied as bearer authorisation
'
const endPoint = "http://ip-api.com/json"
print android.request(endPoint)

