import {LSCSV} from '../../resource/lscsv.js';

test('open csv file', ()=>{
  expect(LSCSV.open('../data/csv/before.csv')).not.toBe(null);
});