USE [dnstaging]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListDailySaleByProduct]    Script Date: 11/14/2026 9:01:06 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 6
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-06-23
description : ?? ? ?? ??
result set : ProductID, ProductName, Quantity, Lifespan, Price, UniqueBuyerCount, UniqueCharacterCount, SaleCount, SaleAmount, NationalityCode
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
4 = input parameter / result set ? NationalityCode ?? - 20110215.wgkim
5 = NationalityCode ??? ?? - 20110304.wgkim
6 = @inyPayMethodCode ?? ?? - 20110304.wgkim
**/

CREATE PROCEDURE [dbo].[P_GetListDailySaleByProduct]
	  @datBasisDate date --// ?? ??
	, @inyPayMethodCode tinyint --// 1=????, 2=??, 3=??, 4=??, 5=?????, 6=??? ??, 7=?? ??, 8=??? ?? ? ??, 9=VIP ??, 10=PVP ?? ???, 11=?? ?? ???, 12=?? ?? ???, 13=?? ?? ???
	, @inyPetalIncomeCode tinyint --// 0=??????, 1=??, 2=??? ??, 3=??? ?? ??, 4=???, 5=??? ??? ?? ??
	, @inyNationalityCode tinyint = NULL --// 1=Afghanistan, 2=Aland Islands, 3=Albania, 4=Algeria, 5=American Samoa, 6=Andorra, 7=Angola, 8=Anguilla, 9=Antarctica, 10=Antigua and Barbuda, 11=Argentina, 12=Armenia, 13=Aruba, 14=Australia, 15=Austria, 16=Azerbaijan, 17=Bahamas, 18=Bahrain, 19=Bangladesh, 20=Barbados, 21=Belarus, 22=Belgium, 23=Belize, 24=Benin, 25=Bermuda, 26=Bhutan, 27=Bolivia, 28=Bosnia and Herzegovina, 29=Botswana, 30=Bouvet Island, 31=Brazil, 32=British Indian Ocean Territory, 33=Brunei Darussalam, 34=Bulgaria, 35=Burkina Faso, 36=Burundi, 37=Cambodia, 38=Cameroon, 39=Canada, 40=Cape Verde, 41=Cayman Islands, 42=Central African Republic, 43=Chad, 44=Chile, 45=China, 46=Colombia, 47=Comoros, 48=Congo, 49=Congo The Democratic Republic of the, 50=Cook Islands, 51=Costa Rica, 52=Cote D'Ivoire, 53=Croatia, 54=Cuba, 55=Cyprus, 56=Czech Republic, 57=Denmark, 58=Djibouti, 59=Dominica, 60=Dominican Republic, 61=Ecuador, 62=Egypt, 63=El Salvador, 64=Equatorial Guinea, 65=Eritrea, 66=Estonia, 67=Ethiopia, 68=Europe, 69=Falkland Islands (Malvinas), 70=Faroe Islands, 71=Fiji, 72=Finland, 73=France, 74=French Guiana, 75=French Polynesia, 76=Gabon, 77=Gambia, 78=Georgia, 79=Germany, 80=Ghana, 81=Gibraltar, 82=Greece, 83=Greenland, 84=Grenada, 85=Guadeloupe, 86=Guam, 87=Guatemala, 88=Guernsey, 89=Guinea, 90=Guinea-Bissau, 91=Guyana, 92=Haiti, 93=Holy See (Vatican City State), 94=Honduras, 95=Hong Kong, 96=Hungary, 97=Iceland, 98=India, 99=Indonesia, 100=Iran Islamic Republic of, 101=Iraq, 102=Ireland, 103=Isle of Man, 104=Israel, 105=Italy, 106=Jamaica, 107=Japan, 108=Jersey, 109=Jordan, 110=Kazakstan, 111=Kenya, 112=Kiribati, 113=Korea Democratic People's Republic of, 114=????, 115=Kuwait, 116=Kyrgyzstan, 117=Lao People's Democratic Republic, 118=Latvia, 119=Lebanon, 120=Lesotho, 121=Liberia, 122=Libyan Arab Jamahiriya, 123=Liechtenstein, 124=Lithuania, 125=Luxembourg, 126=Macau, 127=Macedonia, 128=Madagascar, 129=Malawi, 130=Malaysia, 131=Maldives, 132=Mali, 133=Malta, 134=Marshall Islands, 135=Martinique, 136=Mauritania, 137=Mauritius, 138=Mayotte, 139=Mexico, 140=Micronesia Federated States of, 141=Moldova Republic of, 142=Monaco, 143=Mongolia, 144=Montenegro, 145=Montserrat, 146=Morocco, 147=Mozambique, 148=Myanmar, 149=Namibia, 150=Nauru, 151=Nepal, 152=Netherlands, 153=Netherlands Antilles, 154=New Caledonia, 155=New Zealand, 156=Nicaragua, 157=Niger, 158=Nigeria, 159=Niue, 160=Norfolk Island, 161=Northern Mariana Islands, 162=Norway, 163=Oman, 164=Pakistan, 165=Palau, 166=Palestinian Territory Occupied, 167=Panama, 168=Papua New Guinea, 169=Paraguay, 170=Peru, 171=Philippines, 172=Poland, 173=Portugal, 174=Puerto Rico, 175=Qatar, 176=Reunion, 177=Romania, 178=Russian Federation, 179=Rwanda, 180=Saint Kitts and Nevis, 181=Saint Lucia, 182=Saint Pierre and Miquelon, 183=Saint Vincent and the Grenadines, 184=Samoa, 185=San Marino, 186=Sao Tome and Principe, 187=Satellite Provider, 188=Saudi Arabia, 189=Senegal, 190=Serbia, 191=Seychelles, 192=Sierra Leone, 193=Singapore, 194=Slovakia, 195=Slovenia, 196=Solomon Islands, 197=Somalia, 198=South Africa, 199=Spain, 200=Sri Lanka, 201=Sudan, 202=Suriname, 203=Swaziland, 204=Sweden, 205=Switzerland, 206=Syrian Arab Republic, 207=Taiwan, 208=Tajikistan, 209=Tanzania United Republic of, 210=Thailand, 211=Togo, 212=Tokelau, 213=Tonga, 214=Trinidad and Tobago, 215=Tunisia, 216=Turkey, 217=Turkmenistan, 218=Turks and Caicos Islands, 219=Tuvalu, 220=Uganda, 221=Ukraine, 222=United Arab Emirates, 223=United Kingdom, 224=United States, 225=United States Minor Outlying Islands, 226=Uruguay, 227=Uzbekistan, 228=Vanuatu, 229=Venezuela, 230=Vietnam, 231=Virgin Islands British, 232=Virgin Islands U.S., 233=Wallis and Futuna, 234=Yemen, 235=Zambia, 236=Zimbabwe
WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	SELECT S.ProductID, DP.ProductName, DP.Quantity, DP.Lifespan, DP.Price
		, S.UniqueBuyerCount, S.UniqueCharacterCount, S.SaleCount, S.SaleAmount, S.NationalityCode
	FROM dbo.ST_DailySalesByProduct S WITH (NOLOCK)
		LEFT OUTER JOIN dbo.DimProducts DP WITH (NOLOCK) ON S.ProductID = DP.ProductID
	WHERE BasisDate = @datBasisDate AND S.PayMethodCode = @inyPayMethodCode AND S.PetalIncomeCode = COALESCE(@inyPetalIncomeCode, 0) AND (@inyNationalityCode IS NULL OR S.NationalityCode = @inyNationalityCode)
	ORDER BY S.ProductID, S.NationalityCode;
END TRY
BEGIN CATCH
	GOTO ErrorHandler;
END CATCH;

RETURN 0;

ErrorHandler:
IF XACT_STATE() <> 0
	ROLLBACK TRANSACTION;

IF @intReturnValue IS NULL OR @intReturnValue = 0
	EXEC @intReturnValue = dbo.P_AddErrorLog;

RETURN @intReturnValue;
GO
