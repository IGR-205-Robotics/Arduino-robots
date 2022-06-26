$(document).ready(function(){

	//=================================================
	// Variables
	//=================================================
	var websock;
	var esp_adress = "esp_tester.local";
	var esp_socket_port = "81";

	//=================================================
	// Prefill GUI
	//=================================================
	$(".device_adress").val(esp_adress);

	//=================================================
	// Setup GUI elements
	//=================================================
	$( "#var1_slider" ).slider({
		value:50,
		min:1,
		max:100,
		change: function(event,ui){
									$("#var1").html(ui.value);
									//PUT CALLBACK METHOD HERE
								  }
	});
	
	$( "#var2_slider" ).slider({
		value:50,
		min:1,
		max:100,
		change: function(event,ui){
									$("#var2").html(ui.value);
									//PUT CALLBACK METHOD HERE
								  }
	});
	$( "#var3_slider" ).slider({
		value:50,
		min:1,
		max:100,
		change: function(event,ui){
									$("#var3").html(ui.value);
									//PUT CALLBACK METHOD HERE
								  }
	});
	
	//=================================================
	// Bindings
	//=================================================
	$('.button_with_id').on("click",function(){
		buttonWithIDClicked($(this));
	});
	$('.button1').on("click",function(){
		button1Clicked();
	});
	$('.button2').on("click",function(){
		button2Clicked();
	});
	$('.button3').on("click",function(){
		button3Clicked();
	});
	$('.connect_to_esp').on("click",function(){
		connectToEsp();
	});
	

	//=================================================	 
	// Methods declaration
	//=================================================
	function buttonWithIDClicked(e){
		console.log("button with id", e.attr('id')," clicked");
	};

	function button1Clicked(){
		console.log("button1 clicked");
		var var1 = $("#var1").text();
		var var2 = $("#var2").text();
		var var3 = $("#var3").text();
		$.get( "http://" + esp_adress + "/message",{arg1:var1,arg2:var2,arg3:var3}, function( data ) {});

	};
	function button2Clicked(){
		console.log("button2 clicked");
		websock.send("[10,12,14;11,13,15;12,14,16;]");
	};
	function button3Clicked(){
		console.log("button3 clicked");
	};
	
	function connectToEsp(){
		console.log("connectToEsp");
		esp_adress = $(".device_adress").val();
		start_socket();

	};
	
	//=================================================	 
	// SOCKET
	//=================================================
	function start_socket(){
	
	  var url = "ws://" + esp_adress + ":" + esp_socket_port + "/";
	  websock = new WebSocket(url);
	  console.log("Trying to open websocket at: ", url);

	  websock.onopen = function(evt) { console.log('websocket OPEN!'); };
	  websock.onclose = function(evt) {
			var mess = 'websocket closed!';  
			console.log(mess); 
			var e = document.getElementById('mess_from_ESP');
			e.textContent = mess;
		
	  };
	  websock.onerror = function(evt) { console.log(evt); };
	  websock.onmessage = function(evt) {
		//console.log(evt);
		var e = document.getElementById('mess_from_ESP');
		e.textContent = evt.data.substring(0,50) + "...";
	  };
	};
	
	
});