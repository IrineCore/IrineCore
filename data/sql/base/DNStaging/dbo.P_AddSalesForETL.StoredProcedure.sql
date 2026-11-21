USE [dnstaging]
GO
/****** Object:  StoredProcedure [dbo].[P_AddSalesForETL]    Script Date: 11/14/2026 9:01:06 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 5
author : ???
e-mail : ohtack@eyedentitygames.com
created date : 2011-10-10
description : Sales  ?? ???? ?????
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
4 = ?? ??? ???? ????? ?? by ??? at 2013-03-19
5 = C2C ?? ?? ?? ?? by ??? at 2013-05-22
**/

CREATE PROCEDURE [dbo].[P_AddSalesForETL]
	  @dtmBasisDate datetime --// ?? ??
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
	;WITH PTO AS (
		SELECT PurchaseOrderID, MIN(PetalIncomeID) AS PetalIncomeID
		FROM dbo.PetalOutgo WITH (NOLOCK)
		GROUP BY PurchaseOrderID
	)
	INSERT dbo.ST_DailySalesByCharacter (BasisDate, WorldID, JobCode, CharacterLevel, PayMethodCode, PetalIncomeCode, UniqueCharacterCount, SaleCount, SaleAmount, NationalityCode, UniqueAccountCount)
	SELECT @dtmBasisDate, DC.WorldID, CS.JobCode, CS.CharacterLevel, PO.PayMethodCode, ISNULL(PTI.PetalIncomeCode, 0), COUNT(DISTINCT PO.CharacterID), COUNT(*), SUM(POD.ListPrice), DA.NationalityCode, COUNT(DISTINCT DA.AccountID)
	FROM dbo.PurchaseOrders PO WITH (NOLOCK)
		INNER JOIN dbo.PurchaseOrderDetails POD WITH (NOLOCK) ON PO.PurchaseOrderID = POD.PurchaseOrderID
		INNER JOIN dbo.DailyCharacterStatus CS WITH (NOLOCK) ON PO.CharacterID = CS.CharacterID AND CS.BasisDate = CAST(@dtmBasisDate AS date)
		INNER JOIN dbo.DimCharacters DC WITH (NOLOCK) ON CS.CharacterID = DC.CharacterID AND DC.AccountLevelCode = 0
		LEFT OUTER JOIN PTO ON PO.PurchaseOrderID = PTO.PurchaseOrderID
		LEFT OUTER JOIN dbo.PetalIncome PTI WITH (NOLOCK) ON PTO.PetalIncomeID = PTI.PetalIncomeID
		LEFT OUTER JOIN dbo.DimAccounts DA WITH (NOLOCK) ON DC.AccountID = DA.AccountID
	WHERE PO.OrderDate >= @dtmBasisDate AND PO.OrderDate < DATEADD(DAY, 1, @dtmBasisDate) AND PO.OrderStatusCode = 2
	GROUP BY DC.WorldID, CS.JobCode, CS.CharacterLevel, PO.PayMethodCode, PTI.PetalIncomeCode, DA.NationalityCode;

	;WITH PTO AS (
		SELECT PurchaseOrderID, MIN(PetalIncomeID) AS PetalIncomeID
		FROM dbo.PetalOutgo WITH (NOLOCK)
		GROUP BY PurchaseOrderID
	)
	INSERT dbo.ST_DailySalesByCharacterProduct (BasisDate, ProductID, WorldID, JobCode, CharacterLevel, PayMethodCode, PetalIncomeCode, UniqueCharacterCount, SaleCount, SaleAmount, NationalityCode, UniqueAccountCount)
	SELECT @dtmBasisDate, POD.ProductID, DC.WorldID, CS.JobCode, CS.CharacterLevel, PO.PayMethodCode, ISNULL(PTI.PetalIncomeCode, 0), COUNT(DISTINCT PO.CharacterID), COUNT(*), SUM(POD.ListPrice), DA.NationalityCode, COUNT(DISTINCT DA.AccountID)
	FROM dbo.PurchaseOrders PO WITH (NOLOCK)
		INNER JOIN dbo.PurchaseOrderDetails POD WITH (NOLOCK) ON PO.PurchaseOrderID = POD.PurchaseOrderID
		INNER JOIN dbo.DailyCharacterStatus CS WITH (NOLOCK) ON PO.CharacterID = CS.CharacterID AND CS.BasisDate = CAST(@dtmBasisDate AS date)
		INNER JOIN dbo.DimCharacters DC WITH (NOLOCK) ON CS.CharacterID = DC.CharacterID AND DC.AccountLevelCode = 0
		LEFT OUTER JOIN PTO ON PO.PurchaseOrderID = PTO.PurchaseOrderID
		LEFT OUTER JOIN dbo.PetalIncome PTI WITH (NOLOCK) ON PTO.PetalIncomeID = PTI.PetalIncomeID
		LEFT OUTER JOIN dbo.DimAccounts DA WITH (NOLOCK) ON DC.AccountID = DA.AccountID
	WHERE PO.OrderDate >= @dtmBasisDate AND PO.OrderDate < DATEADD(DAY, 1, @dtmBasisDate) AND PO.OrderStatusCode = 2
	GROUP BY POD.ProductID, DC.WorldID, CS.JobCode, CS.CharacterLevel, PO.PayMethodCode, PTI.PetalIncomeCode, DA.NationalityCode;

	;WITH PTO AS (
		SELECT PurchaseOrderID, MIN(PetalIncomeID) AS PetalIncomeID
		FROM dbo.PetalOutgo WITH (NOLOCK)
		GROUP BY PurchaseOrderID
	)
	INSERT dbo.ST_DailySalesByProduct (BasisDate, ProductID, PayMethodCode, PetalIncomeCode, UniqueBuyerCount, UniqueCharacterCount, SaleCount, SaleAmount, NationalityCode)
	SELECT @dtmBasisDate, POD.ProductID, PO.PayMethodCode, ISNULL(PTI.PetalIncomeCode, 0), COUNT(DISTINCT CH.AccountID), COUNT(DISTINCT PO.CharacterID), COUNT(*), SUM(POD.ListPrice), DA.NationalityCode
	FROM dbo.PurchaseOrders PO WITH (NOLOCK)
		INNER JOIN dbo.PurchaseOrderDetails POD WITH (NOLOCK) ON PO.PurchaseOrderID = POD.PurchaseOrderID
		INNER JOIN dbo.DimCharacters CH WITH (NOLOCK) ON PO.CharacterID = CH.CharacterID AND CH.AccountLevelCode = 0
		LEFT OUTER JOIN PTO ON PO.PurchaseOrderID = PTO.PurchaseOrderID
		LEFT OUTER JOIN dbo.PetalIncome PTI WITH (NOLOCK) ON PTO.PetalIncomeID = PTI.PetalIncomeID
		LEFT OUTER JOIN dbo.DimAccounts DA WITH (NOLOCK) ON CH.AccountID = DA.AccountID
	WHERE PO.OrderDate >= @dtmBasisDate AND PO.OrderDate < DATEADD(DAY, 1, @dtmBasisDate) AND PO.OrderStatusCode = 2
	GROUP BY POD.ProductID, PO.PayMethodCode, PTI.PetalIncomeCode, DA.NationalityCode;

	;WITH PTO AS (
		SELECT PurchaseOrderID, MIN(PetalIncomeID) AS PetalIncomeID
		FROM dbo.PetalOutgo WITH (NOLOCK)
		GROUP BY PurchaseOrderID
	), C1 AS (
		SELECT DC.AccountID, PO.PayMethodCode, ISNULL(PTI.PetalIncomeCode, 0) AS PetalIncomeCode, SUM(POD.ListPrice) AS Price, DA.NationalityCode
		FROM dbo.PurchaseOrders PO
		INNER JOIN dbo.PurchaseOrderDetails POD WITH (NOLOCK) ON PO.PurchaseOrderID = POD.PurchaseOrderID
			INNER JOIN dbo.DimCharacters DC ON PO.CharacterID = DC.CharacterID AND DC.AccountLevelCode = 0
			LEFT OUTER JOIN PTO ON PO.PurchaseOrderID = PTO.PurchaseOrderID
			LEFT OUTER JOIN dbo.PetalIncome PTI WITH (NOLOCK) ON PTO.PetalIncomeID = PTI.PetalIncomeID
			LEFT OUTER JOIN dbo.DimAccounts DA WITH (NOLOCK) ON DC.AccountID = DA.AccountID
		WHERE PO.OrderDate >= @dtmBasisDate AND PO.OrderDate < DATEADD(DAY, 1, @dtmBasisDate) AND PO.OrderStatusCode = 2
		GROUP BY DC.AccountID, PO.PayMethodCode, PTI.PetalIncomeCode, DA.NationalityCode
	)
	INSERT dbo.ST_DailySalesAmountRange (BasisDate, SaleAmountRangeCode, PayMethodCode, PetalIncomeCode, UniqueBuyerCount, SaleCount, SaleAmount, NationalityCode)
	SELECT @dtmBasisDate, 
		CASE
			WHEN Price < 1000 THEN 1
			WHEN Price BETWEEN 1000 AND 4999 THEN 2
			WHEN Price BETWEEN 5000 AND 9999 THEN 3
			WHEN Price BETWEEN 10000 AND 19999 THEN 4
			WHEN Price BETWEEN 20000 AND 29999 THEN 5
			WHEN Price BETWEEN 30000 AND 39999 THEN 6
			WHEN Price BETWEEN 40000 AND 49999 THEN 7
			WHEN Price BETWEEN 50000 AND 99999 THEN 8
			WHEN Price BETWEEN 100000 AND 199999 THEN 9
			WHEN Price BETWEEN 200000 AND 299999 THEN 10
			WHEN Price >= 300000 THEN 11
		END
		, PayMethodCode, PetalIncomeCode, COUNT(DISTINCT AccountID), COUNT(*), SUM(Price), NationalityCode
	FROM C1
	GROUP BY 
		CASE
			WHEN Price < 1000 THEN 1
			WHEN Price BETWEEN 1000 AND 4999 THEN 2
			WHEN Price BETWEEN 5000 AND 9999 THEN 3
			WHEN Price BETWEEN 10000 AND 19999 THEN 4
			WHEN Price BETWEEN 20000 AND 29999 THEN 5
			WHEN Price BETWEEN 30000 AND 39999 THEN 6
			WHEN Price BETWEEN 40000 AND 49999 THEN 7
			WHEN Price BETWEEN 50000 AND 99999 THEN 8
			WHEN Price BETWEEN 100000 AND 199999 THEN 9
			WHEN Price BETWEEN 200000 AND 299999 THEN 10
			WHEN Price >= 300000 THEN 11
		END, PayMethodCode, PetalIncomeCode, NationalityCode;

	INSERT dbo.ST_DailyC2CByCharacter (BasisDate, WorldID, JobCode, CharacterLevel, UniqueCharacterCount, SaleCount, SaleAmount, NationalityCode, UniqueAccountCount)
	SELECT @dtmBasisDate, DC.WorldID, CS.JobCode, CS.CharacterLevel, COUNT(DISTINCT GMT.ToCharacterID), COUNT(*), SUM(GMT.PaidCashAmount), DA.NationalityCode, COUNT(DISTINCT DA.AccountID)
	FROM dbo.GameMoneyTrade GMT WITH(NOLOCK)
		INNER JOIN dbo.DailyCharacterStatus CS WITH (NOLOCK) ON GMT.ToCharacterID = CS.CharacterID AND CS.BasisDate = CAST(@dtmBasisDate AS date)
		INNER JOIN dbo.DimCharacters DC WITH (NOLOCK) ON CS.CharacterID = DC.CharacterID AND DC.AccountLevelCode = 0
		LEFT OUTER JOIN dbo.DimAccounts DA WITH (NOLOCK) ON DC.AccountID = DA.AccountID
	WHERE TradeDate >= @dtmBasisDate AND TradeDate < DATEADD(DAY, 1, @dtmBasisDate)
	GROUP BY DC.WorldID, CS.JobCode, CS.CharacterLevel, DA.NationalityCode;

	IF DATEPART(DAY, DATEADD(DAY, 1, @dtmBasisDate)) = 1
	BEGIN
		;WITH PTO AS (
			SELECT PurchaseOrderID, MIN(PetalIncomeID) AS PetalIncomeID
			FROM dbo.PetalOutgo WITH (NOLOCK)
			GROUP BY PurchaseOrderID
		)
		INSERT dbo.ST_MonthlySalesByCharacter (BasisDate, WorldID, JobCode, CharacterLevel, PayMethodCode, PetalIncomeCode, UniqueCharacterCount, SaleCount, SaleAmount, NationalityCode, UniqueAccountCount)
		SELECT DATEADD(MONTH, DATEDIFF(MONTH, 0, @dtmBasisDate), 0), DC.WorldID, CS.JobCode, CS.CharacterLevel, PO.PayMethodCode, ISNULL(PTI.PetalIncomeCode, 0), COUNT(DISTINCT PO.CharacterID), COUNT(*), SUM(POD.ListPrice), DA.NationalityCode, COUNT(DISTINCT DA.AccountID)
		FROM dbo.PurchaseOrders PO WITH (NOLOCK)
			INNER JOIN dbo.PurchaseOrderDetails POD WITH (NOLOCK) ON PO.PurchaseOrderID = POD.PurchaseOrderID
			INNER JOIN dbo.DailyCharacterStatus CS WITH (NOLOCK) ON PO.CharacterID = CS.CharacterID AND CS.BasisDate = CAST(@dtmBasisDate AS date)
			INNER JOIN dbo.DimCharacters DC WITH (NOLOCK) ON CS.CharacterID = DC.CharacterID AND DC.AccountLevelCode = 0
			LEFT OUTER JOIN PTO ON PO.PurchaseOrderID = PTO.PurchaseOrderID
			LEFT OUTER JOIN dbo.PetalIncome PTI WITH (NOLOCK) ON PTO.PetalIncomeID = PTI.PetalIncomeID
			LEFT OUTER JOIN dbo.DimAccounts DA WITH (NOLOCK) ON DC.AccountID = DA.AccountID
		WHERE PO.OrderDate >= DATEADD(MONTH, DATEDIFF(MONTH, 0, @dtmBasisDate), 0) AND PO.OrderDate < DATEADD(DAY, 1, @dtmBasisDate) AND PO.OrderStatusCode = 2
		GROUP BY DC.WorldID, CS.JobCode, CS.CharacterLevel, PO.PayMethodCode, PTI.PetalIncomeCode, DA.NationalityCode;

		;WITH PTO AS (
			SELECT PurchaseOrderID, MIN(PetalIncomeID) AS PetalIncomeID
			FROM dbo.PetalOutgo WITH (NOLOCK)
			GROUP BY PurchaseOrderID
		)
		INSERT dbo.ST_MonthlySalesByCharacterProduct (BasisDate, ProductID, WorldID, JobCode, CharacterLevel, PayMethodCode, PetalIncomeCode, UniqueCharacterCount, SaleCount, SaleAmount, NationalityCode, UniqueAccountCount)
		SELECT DATEADD(MONTH, DATEDIFF(MONTH, 0, @dtmBasisDate), 0), POD.ProductID, DC.WorldID, CS.JobCode, CS.CharacterLevel, PO.PayMethodCode, ISNULL(PTI.PetalIncomeCode, 0), COUNT(DISTINCT PO.CharacterID), COUNT(*), SUM(POD.ListPrice), DA.NationalityCode, COUNT(DISTINCT DA.AccountID)
		FROM dbo.PurchaseOrders PO WITH (NOLOCK)
			INNER JOIN dbo.PurchaseOrderDetails POD WITH (NOLOCK) ON PO.PurchaseOrderID = POD.PurchaseOrderID
			INNER JOIN dbo.DailyCharacterStatus CS WITH (NOLOCK) ON PO.CharacterID = CS.CharacterID AND CS.BasisDate = CAST(@dtmBasisDate AS date)
			INNER JOIN dbo.DimCharacters DC WITH (NOLOCK) ON CS.CharacterID = DC.CharacterID AND DC.AccountLevelCode = 0
			LEFT OUTER JOIN PTO ON PO.PurchaseOrderID = PTO.PurchaseOrderID
			LEFT OUTER JOIN dbo.PetalIncome PTI WITH (NOLOCK) ON PTO.PetalIncomeID = PTI.PetalIncomeID
			LEFT OUTER JOIN dbo.DimAccounts DA WITH (NOLOCK) ON DC.AccountID = DA.AccountID
		WHERE PO.OrderDate >= DATEADD(MONTH, DATEDIFF(MONTH, 0, @dtmBasisDate), 0) AND PO.OrderDate < DATEADD(DAY, 1, @dtmBasisDate) AND PO.OrderStatusCode = 2
		GROUP BY POD.ProductID, DC.WorldID, CS.JobCode, CS.CharacterLevel, PO.PayMethodCode, PTI.PetalIncomeCode, DA.NationalityCode;

		;WITH PTO AS (
			SELECT PurchaseOrderID, MIN(PetalIncomeID) AS PetalIncomeID
			FROM dbo.PetalOutgo WITH (NOLOCK)
			GROUP BY PurchaseOrderID
		)
		INSERT dbo.ST_MonthlySalesByProduct (BasisDate, ProductID, PayMethodCode, PetalIncomeCode, UniqueBuyerCount, UniqueCharacterCount, SaleCount, SaleAmount, NationalityCode)
		SELECT DATEADD(MONTH, DATEDIFF(MONTH, 0, @dtmBasisDate), 0), POD.ProductID, PO.PayMethodCode, ISNULL(PTI.PetalIncomeCode, 0), COUNT(DISTINCT CH.AccountID), COUNT(DISTINCT PO.CharacterID), COUNT(*), SUM(POD.ListPrice), DA.NationalityCode
		FROM dbo.PurchaseOrders PO WITH (NOLOCK)
			INNER JOIN dbo.PurchaseOrderDetails POD WITH (NOLOCK) ON PO.PurchaseOrderID = POD.PurchaseOrderID
			INNER JOIN dbo.DimCharacters CH WITH (NOLOCK) ON PO.CharacterID = CH.CharacterID AND CH.AccountLevelCode = 0
			LEFT OUTER JOIN PTO ON PO.PurchaseOrderID = PTO.PurchaseOrderID
			LEFT OUTER JOIN dbo.PetalIncome PTI WITH (NOLOCK) ON PTO.PetalIncomeID = PTI.PetalIncomeID
			LEFT OUTER JOIN dbo.DimAccounts DA WITH (NOLOCK) ON CH.AccountID = DA.AccountID
		WHERE PO.OrderDate >= DATEADD(MONTH, DATEDIFF(MONTH, 0, @dtmBasisDate), 0) AND PO.OrderDate < DATEADD(DAY, 1, @dtmBasisDate) AND PO.OrderStatusCode = 2
		GROUP BY POD.ProductID, PO.PayMethodCode, PTI.PetalIncomeCode, DA.NationalityCode;

		;WITH PTO AS (
			SELECT PurchaseOrderID, MIN(PetalIncomeID) AS PetalIncomeID
			FROM dbo.PetalOutgo WITH (NOLOCK)
			GROUP BY PurchaseOrderID
		), C1 AS (
			SELECT DC.AccountID, PO.PayMethodCode, ISNULL(PTI.PetalIncomeCode, 0) AS PetalIncomeCode, SUM(POD.ListPrice) AS Price, DA.NationalityCode
			FROM dbo.PurchaseOrders PO
				INNER JOIN dbo.DimCharacters DC ON PO.CharacterID = DC.CharacterID AND DC.AccountLevelCode = 0
				INNER JOIN dbo.PurchaseOrderDetails POD WITH (NOLOCK) ON PO.PurchaseOrderID = POD.PurchaseOrderID
				LEFT OUTER JOIN PTO ON PO.PurchaseOrderID = PTO.PurchaseOrderID
				LEFT OUTER JOIN dbo.PetalIncome PTI WITH (NOLOCK) ON PTO.PetalIncomeID = PTI.PetalIncomeID
				LEFT OUTER JOIN dbo.DimAccounts DA WITH (NOLOCK) ON DC.AccountID = DA.AccountID
			WHERE PO.OrderDate >= DATEADD(MONTH, DATEDIFF(MONTH, 0, @dtmBasisDate), 0) AND PO.OrderDate < DATEADD(DAY, 1, @dtmBasisDate) AND PO.OrderStatusCode = 2
			GROUP BY DC.AccountID, PO.PayMethodCode, PTI.PetalIncomeCode, DA.NationalityCode
		)
		INSERT dbo.ST_MonthlySalesAmountRange (BasisDate, SaleAmountRangeCode, PayMethodCode, PetalIncomeCode, UniqueBuyerCount, SaleCount, SaleAmount, NationalityCode)
		SELECT DATEADD(MONTH, DATEDIFF(MONTH, 0, @dtmBasisDate), 0), 
			CASE
				WHEN Price < 1000 THEN 1
				WHEN Price BETWEEN 1000 AND 4999 THEN 2
				WHEN Price BETWEEN 5000 AND 9999 THEN 3
				WHEN Price BETWEEN 10000 AND 19999 THEN 4
				WHEN Price BETWEEN 20000 AND 29999 THEN 5
				WHEN Price BETWEEN 30000 AND 39999 THEN 6
				WHEN Price BETWEEN 40000 AND 49999 THEN 7
				WHEN Price BETWEEN 50000 AND 99999 THEN 8
				WHEN Price BETWEEN 100000 AND 199999 THEN 9
				WHEN Price BETWEEN 200000 AND 299999 THEN 10
				WHEN Price >= 300000 THEN 11
			END
			, PayMethodCode, PetalIncomeCode, COUNT(DISTINCT AccountID), COUNT(*), SUM(Price), NationalityCode
		FROM C1
		GROUP BY 
			CASE
				WHEN Price < 1000 THEN 1
				WHEN Price BETWEEN 1000 AND 4999 THEN 2
				WHEN Price BETWEEN 5000 AND 9999 THEN 3
				WHEN Price BETWEEN 10000 AND 19999 THEN 4
				WHEN Price BETWEEN 20000 AND 29999 THEN 5
				WHEN Price BETWEEN 30000 AND 39999 THEN 6
				WHEN Price BETWEEN 40000 AND 49999 THEN 7
				WHEN Price BETWEEN 50000 AND 99999 THEN 8
				WHEN Price BETWEEN 100000 AND 199999 THEN 9
				WHEN Price BETWEEN 200000 AND 299999 THEN 10
				WHEN Price >= 300000 THEN 11
			END, PayMethodCode, PetalIncomeCode, NationalityCode;

		INSERT dbo.ST_MonthlyC2CByCharacter (BasisDate, WorldID, JobCode, CharacterLevel, UniqueCharacterCount, SaleCount, SaleAmount, NationalityCode, UniqueAccountCount)
		SELECT DATEADD(MONTH, DATEDIFF(MONTH, 0, @dtmBasisDate), 0), DC.WorldID, CS.JobCode, CS.CharacterLevel, COUNT(DISTINCT GMT.ToCharacterID), COUNT(*), SUM(GMT.PaidCashAmount), DA.NationalityCode, COUNT(DISTINCT DA.AccountID)
		FROM dbo.GameMoneyTrade GMT WITH(NOLOCK)
			INNER JOIN dbo.DailyCharacterStatus CS WITH (NOLOCK) ON GMT.ToCharacterID = CS.CharacterID AND CS.BasisDate = CAST(@dtmBasisDate AS date)
			INNER JOIN dbo.DimCharacters DC WITH (NOLOCK) ON CS.CharacterID = DC.CharacterID AND DC.AccountLevelCode = 0
			LEFT OUTER JOIN dbo.DimAccounts DA WITH (NOLOCK) ON DC.AccountID = DA.AccountID
		WHERE TradeDate >= DATEADD(MONTH, DATEDIFF(MONTH, 0, @dtmBasisDate), 0) AND TradeDate < DATEADD(DAY, 1, @dtmBasisDate)
	GROUP BY DC.WorldID, CS.JobCode, CS.CharacterLevel, DA.NationalityCode;

	END
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
