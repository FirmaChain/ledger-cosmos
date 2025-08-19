/** ******************************************************************************
 *  (c) 2018 - 2024 Zondax AG
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 ******************************************************************************* */

import Zemu, { ClickNavigation, TouchNavigation, isTouchDevice } from '@zondax/zemu'
// @ts-ignore
import CosmosApp from '@zondax/ledger-cosmos-js'
import { defaultOptions, DEVICE_MODELS, tx_sign_textual, TEXTUAL_TX } from './common'
// @ts-ignore
import secp256k1 from 'secp256k1/elliptic'
// @ts-ignore
import crypto from 'crypto'
import { ButtonKind, IButton, SwipeDirection } from '@zondax/zemu/dist/types'

jest.setTimeout(90000)

// Textual mode is not available for NanoS
const TEXTUAL_MODELS = DEVICE_MODELS.filter(m => m.name !== 'nanos')

describe('Textual', function () {
  // eslint-disable-next-line jest/expect-expect
  test.concurrent.each(TEXTUAL_MODELS)('can start and stop container', async function (m) {
    const sim = new Zemu(m.path)
    try {
      await sim.start({ ...defaultOptions, model: m.name })
    } finally {
      await sim.close()
    }
  })

  test.concurrent.each(TEXTUAL_MODELS)('sign basic textual', async function (m) {
    const sim = new Zemu(m.path)
    try {
      await sim.start({ ...defaultOptions, model: m.name })
      const app = new CosmosApp(sim.getTransport())

      const path = "m/44'/7777777'/0'/0/0"
      const tx = Buffer.from(tx_sign_textual, 'hex')
      const hrp = 'firma'

      // get address / publickey
      const respPk = await app.getAddressAndPubKey(path, hrp)
      expect(respPk).toHaveProperty('compressed_pk')
      expect(respPk).toHaveProperty('bech32_address')
      console.log(respPk)

      // do not wait here..
      const signatureRequest = app.sign(path, tx, hrp, TEXTUAL_TX)

      // Wait until we are not in the main menu
      await sim.waitUntilScreenIsNot(sim.getMainMenuSnapshot())
      await sim.compareSnapshotsAndApprove('.', `${m.prefix.toLowerCase()}-textual-sign_basic`)

      const resp = await signatureRequest
      console.log(resp)

      expect(resp).toHaveProperty('signature')

      // Now verify the signature
      const hash = crypto.createHash('sha256')
      const msgHash = Uint8Array.from(hash.update(tx).digest())

      const signatureDER = resp.signature
      const signature = secp256k1.signatureImport(Uint8Array.from(signatureDER))

      const pk = Uint8Array.from(respPk.compressed_pk)

      const signatureOk = secp256k1.ecdsaVerify(signature, msgHash, pk)
      expect(signatureOk).toEqual(true)
    } finally {
      await sim.close()
    }
  })

  test.concurrent.each(TEXTUAL_MODELS)('sign basic textual expert', async function (m) {
    const sim = new Zemu(m.path)
    try {
      await sim.start({ ...defaultOptions, model: m.name })
      const app = new CosmosApp(sim.getTransport())

      // Change to expert mode so we can skip fields
      await sim.toggleExpertMode()

      const path = "m/44'/7777777'/0'/0/0"
      const tx = Buffer.from(tx_sign_textual, 'hex')
      const hrp = 'firma'

      // get address / publickey
      const respPk = await app.getAddressAndPubKey(path, hrp)
      expect(respPk).toHaveProperty('compressed_pk')
      expect(respPk).toHaveProperty('bech32_address')
      console.log(respPk)

      // do not wait here..
      const signatureRequest = app.sign(path, tx, hrp, TEXTUAL_TX)

      // Wait until we are not in the main menu
      await sim.waitUntilScreenIsNot(sim.getMainMenuSnapshot())
      await sim.compareSnapshotsAndApprove('.', `${m.prefix.toLowerCase()}-textual-sign_basic_expert`)

      const resp = await signatureRequest
      console.log(resp)

      expect(resp).toHaveProperty('signature')

      // Now verify the signature
      const hash = crypto.createHash('sha256')
      const msgHash = Uint8Array.from(hash.update(tx).digest())

      const signatureDER = resp.signature
      const signature = secp256k1.signatureImport(Uint8Array.from(signatureDER))

      const pk = Uint8Array.from(respPk.compressed_pk)

      const signatureOk = secp256k1.ecdsaVerify(signature, msgHash, pk)
      expect(signatureOk).toEqual(true)
    } finally {
      await sim.close()
    }
  })
})