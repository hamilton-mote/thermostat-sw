package main

import (
	//"flag"
	"fmt"
	"log"
	//"os"
	"strings"
	"time"

	"github.com/paypal/gatt"
	"github.com/paypal/gatt/examples/option"
)

var done = make(chan struct{})
var tstat = new(Thermostat)

func onStateChanged(d gatt.Device, s gatt.State) {
	fmt.Println("State:", s)
	switch s {
	case gatt.StatePoweredOn:
		fmt.Println("Scanning...")
		d.Scan([]gatt.UUID{}, false)
		return
	default:
		d.StopScanning()
	}
}

const id = "C0:98:E5:80:80:80"

func onPeriphDiscovered(p gatt.Peripheral, a *gatt.Advertisement, rssi int) {
	if strings.ToUpper(p.ID()) != id {
		return
	}

	// Stop scanning once we've got the peripheral we're looking for.
	p.Device().StopScanning()

	fmt.Printf("\nPeripheral ID:%s, NAME:(%s)\n", p.ID(), p.Name())
	fmt.Println("  Local Name        =", a.LocalName)
	fmt.Println("  TX Power Level    =", a.TxPowerLevel)
	fmt.Println("  Manufacturer Data =", a.ManufacturerData)
	fmt.Println("  Service Data      =", a.ServiceData)
	fmt.Println("")

	p.Device().Connect(p)
}

type Thermostat struct {
	p          gatt.Peripheral
	svc        *gatt.Service
	tstat_char *gatt.Characteristic
}

func onPeriphConnected(p gatt.Peripheral, err error) {
	tstat.p = p

	fmt.Println("Connected")
	defer p.Device().CancelConnection(p)

	if err := p.SetMTU(500); err != nil {
		fmt.Printf("Failed to set MTU, err: %s\n", err)
	}

	// Discovery services
	svc_uuid := gatt.MustParseUUID("7bad890f-2883-4587-e64e-ea96a0dea487")
	ss, err := p.DiscoverServices([]gatt.UUID{svc_uuid})
	if err != nil {
		fmt.Printf("Failed to discover services, err: %s\n", err)
		return
	}

	for _, s := range ss {
		if !s.UUID().Equal(svc_uuid) {
			continue
		}

		tstat.svc = s
		break
	}

	fmt.Printf("svc %+v", tstat.svc)
	char_uuid := gatt.MustParseUUID("7bad8911-2883-4587-e64e-ea96a0dea487")
	cs, err := tstat.p.DiscoverCharacteristics([]gatt.UUID{char_uuid}, tstat.svc)
	if err != nil {
		fmt.Printf("Failed to discover characteristics, err: %s\n", err)
		return
	}
	for _, c := range cs {
		if !c.UUID().Equal(char_uuid) {
			fmt.Println(c.UUID().String())
			continue
		}
		tstat.tstat_char = c
		break
	}

	p.SetIndicateValue(tstat.tstat_char, func(c *gatt.Characteristic, b []byte, err error) {
		fmt.Println(c, b, err)
	})

	// Read the characteristic, if possible.
	for _ = range time.Tick(5 * time.Second) {
		if (tstat.tstat_char.Properties() & gatt.CharRead) != 0 {
			b, err := p.ReadCharacteristic(tstat.tstat_char)
			if err != nil {
				fmt.Printf("Failed to read characteristic, err: %s\n", err)
				return
			}
			temp_in := uint8(b[0])
			temp_hsp := uint8(b[1])
			temp_csp := uint8(b[2])
			state := uint8(b[3])
			hold_timer := uint16(b[4]<<8 | b[5])
			is_heating := (state & 0x08) > 0
			is_cooling := (state & 0x04) > 0
			is_fan_on := (state & 0x02) > 0
			is_tstat_on := (state & 0x01) > 0
			fmt.Printf("Temp: %d. [HSP, CSP]: %d, %d\n", temp_in, temp_hsp, temp_csp)
			fmt.Printf("Tstat? %v Heat? %v Cool? %v Fan? %v Timer: %d\n", is_tstat_on, is_heating, is_cooling, is_fan_on, hold_timer)
		}
	}

	return
}

func onPeriphDisconnected(p gatt.Peripheral, err error) {
	fmt.Println("Disconnected")
	close(done)
}

func main() {

	d, err := gatt.NewDevice(option.DefaultClientOptions...)
	if err != nil {
		log.Fatalf("Failed to open device, err: %s\n", err)
		return
	}

	// Register handlers.
	d.Handle(
		gatt.PeripheralDiscovered(onPeriphDiscovered),
		gatt.PeripheralConnected(onPeriphConnected),
		gatt.PeripheralDisconnected(onPeriphDisconnected),
	)

	d.Init(onStateChanged)
	<-done
	fmt.Println("Done")
}
