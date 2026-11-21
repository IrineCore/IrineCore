USE [dnstaging]
GO
/****** Object:  StoredProcedure [dbo].[P_ReadyToDailyETLStart]    Script Date: 11/14/2026 9:01:06 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 22
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-05-04
description : Daily ETL ?? ??? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
10 = ?? ??? ??? ?? ???? DROP ???. at 2010-08-03
11 = ???? ??? ?? - ST_JobAndLevelType6 at 2010-08-09
12 = sub sp ?? ?? ?? ?? at 2010-08-13
15 = ST_SurvivalRate ?? ??     by 20110425.wgkim
16 = ST_SurvivalRate ?? ??     by 20110425.wgkim
17 = ST_DailyNestClear ?? ??     by ??? at 2011-12-06
17 = ST_DailyVisit ?? ??     by ??? at 2012-09-19
**/

CREATE PROCEDURE [dbo].[P_ReadyToDailyETLStart]
	  @dtmBasisDate datetime --// ?? ??
WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;
SET DATEFIRST 7;

DECLARE @intReturnValue int, @nvcStmt nvarchar(max) = N'';

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	SELECT @nvcStmt += N'DROP TABLE dbo.' + name + N';' FROM sys.tables WHERE name LIKE N'%' + CONVERT(nvarchar(8), @dtmBasisDate, 112);

	EXEC sp_executesql @nvcStmt;

	DELETE dbo.CUCountLogs WHERE LogDate >= @dtmBasisDate AND LogDate < (@dtmBasisDate + 1);

	DELETE dbo.DimAccounts WHERE RegisterDate >= CAST(@dtmBasisDate AS datetime2(0)) AND RegisterDate < CAST(@dtmBasisDate + 1 AS datetime2(0));

	DELETE dbo.PurchaseOrders WHERE OrderDate >= CAST(@dtmBasisDate AS datetime2(0));

	DELETE dbo.PurchaseOrderDetails
	FROM dbo.PurchaseOrderDetails A
	WHERE NOT EXISTS (
		SELECT * FROM dbo.PurchaseOrders WHERE PurchaseOrderID = A.PurchaseOrderID
	);

	DELETE dbo.SelectedItems
	FROM dbo.SelectedItems A
	WHERE NOT EXISTS (
		SELECT * FROM dbo.PurchaseOrderDetails WHERE PurchaseOrderDetailID = A.PurchaseOrderDetailID
	);

	DELETE dbo.SelectedItems
	FROM dbo.PurchaseOrders A
		INNER JOIN dbo.PurchaseOrderDetails B ON A.PurchaseOrderID = B.PurchaseOrderID
		INNER JOIN dbo.SelectedItems C ON B.PurchaseOrderDetailID = C.PurchaseOrderDetailID
	WHERE A.OrderDate >= CAST(@dtmBasisDate AS datetime2(0));

	DELETE dbo.PurchaseOrderDetails
	FROM dbo.PurchaseOrders A
		INNER JOIN dbo.PurchaseOrderDetails B ON A.PurchaseOrderID = B.PurchaseOrderID
	WHERE A.OrderDate >= CAST(@dtmBasisDate AS datetime2(0));

	DELETE dbo.PurchaseOrders WHERE OrderDate >= CAST(@dtmBasisDate AS datetime2(0));

	DELETE dbo.PetalIncome WHERE IncomeDate >= CAST(@dtmBasisDate AS datetime2(0)) AND IncomeDate < CAST(@dtmBasisDate + 1 AS datetime2(0));

	DELETE dbo.PetalOutgo WHERE OutgoDate >= CAST(@dtmBasisDate AS datetime2(0)) AND OutgoDate < CAST(@dtmBasisDate + 1 AS datetime2(0));
	DELETE dbo.DailyUVAccounts WHERE BasisDate = CAST(@dtmBasisDate AS date);

	IF DATEPART(WEEKDAY, @dtmBasisDate) = 7
		DELETE dbo.WeeklyUVAccounts WHERE BasisDate BETWEEN DATEADD(DAY, -6, @dtmBasisDate) AND @dtmBasisDate

	IF DATEPART(DAY, DATEADD(DAY, 1, @dtmBasisDate)) = 1
		DELETE dbo.MonthlyUVAccounts WHERE BasisDate BETWEEN DATEADD(MONTH, DATEDIFF(MONTH, 0, @dtmBasisDate), 0) AND @dtmBasisDate

	DELETE dbo.ST_DailyGameMoney WHERE BasisDate = CAST(@dtmBasisDate AS date);

	DELETE dbo.ST_DailyMaxCU WHERE BasisDate >= CAST(@dtmBasisDate AS smalldatetime) AND BasisDate < CAST(@dtmBasisDate + 1 AS smalldatetime);

	DELETE dbo.ST_HourlyMaxCU WHERE BasisDate >= CAST(@dtmBasisDate AS smalldatetime) AND BasisDate < CAST(@dtmBasisDate + 1 AS smalldatetime);

	DELETE dbo.ST_JobAndLevelType1 WHERE BasisDate = CAST(@dtmBasisDate AS date);

	DELETE dbo.ST_JobAndLevelType2 WHERE BasisDate = CAST(@dtmBasisDate AS date);

	DELETE dbo.ST_JobAndLevelType3 WHERE BasisDate = CAST(@dtmBasisDate AS date);
	DELETE dbo.ST_JobAndLevelType4 WHERE BasisDate = CAST(@dtmBasisDate AS date);
	DELETE dbo.ST_JobAndLevelType5 WHERE BasisDate = CAST(@dtmBasisDate AS date);
	DELETE dbo.ST_JobAndLevelType6 WHERE BasisDate = CAST(@dtmBasisDate AS date);

	DELETE dbo.ST_DailyDashboard WHERE BasisDate = CAST(@dtmBasisDate AS date);

	DELETE dbo.ST_DailySalesByCharacter WHERE BasisDate = CAST(@dtmBasisDate AS date);
	DELETE dbo.ST_MonthlySalesByCharacter WHERE BasisDate = DATEADD(MONTH, DATEDIFF(MONTH, 0, @dtmBasisDate), 0);

	DELETE dbo.ST_DailySalesByCharacterProduct WHERE BasisDate = CAST(@dtmBasisDate AS date);
	DELETE dbo.ST_MonthlySalesByCharacterProduct WHERE BasisDate = DATEADD(MONTH, DATEDIFF(MONTH, 0, @dtmBasisDate), 0);

	DELETE dbo.ST_DailySalesByProduct WHERE BasisDate = CAST(@dtmBasisDate AS date);
	DELETE dbo.ST_MonthlySalesByProduct WHERE BasisDate = DATEADD(MONTH, DATEDIFF(MONTH, 0, @dtmBasisDate), 0);

	DELETE dbo.ST_DailySalesAmountRange WHERE BasisDate = CAST(@dtmBasisDate AS date);
	DELETE dbo.ST_MonthlySalesAmountRange WHERE BasisDate = DATEADD(MONTH, DATEDIFF(MONTH, 0, @dtmBasisDate), 0);
	
	DELETE dbo.ST_DailyC2CByCharacter WHERE BasisDate = CAST(@dtmBasisDate AS date);
	DELETE dbo.ST_MonthlyC2CByCharacter WHERE BasisDate = DATEADD(MONTH, DATEDIFF(MONTH, 0, @dtmBasisDate), 0);

	DELETE dbo.Mon_GameMoney1 WHERE LogDate >= @dtmBasisDate;

	DELETE dbo.Mon_GameMoney2 WHERE BasisDate = CAST(@dtmBasisDate AS date);

	DELETE dbo.ST_ItemSales WHERE BasisDate = CAST(@dtmBasisDate AS date);

	DELETE dbo.ST_ActiveGameMoney WHERE BasisDate = CAST(@dtmBasisDate AS date);

	DELETE dbo.ST_ActiveGameMoneyByCharacterLevel WHERE BasisDate = CAST(@dtmBasisDate AS date);

	DELETE dbo.ST_SurvivalRate WHERE MainCategory = 2 AND SubCategory = 1 AND BasisDate = DATEADD(MM, DATEDIFF(MM, 0, @dtmBasisDate), 0);
	DELETE dbo.ST_SurvivalRate WHERE MainCategory = 2 AND SubCategory = 2 AND BasisDate = DATEADD(MM, DATEDIFF(MM, 0, DATEADD(MONTH, -1, @dtmBasisDate)), 0);
	DELETE dbo.ST_SurvivalRate WHERE MainCategory = 2 AND SubCategory = 3 AND BasisDate = DATEADD(MM, DATEDIFF(MM, 0, @dtmBasisDate), 0);
	DELETE dbo.ST_SurvivalRate WHERE MainCategory = 2 AND SubCategory = 4 AND BasisDate = DATEADD(MM, DATEDIFF(MM, 0, DATEADD(MONTH, -1, @dtmBasisDate)), 0);

	DELETE dbo.ST_DailyNestClear WHERE BasisDate = CAST(@dtmBasisDate AS date);

	DELETE dbo.ST_DailyVisit WHERE BasisDate = CAST(@dtmBasisDate AS date);
	
	DELETE dbo.ST_DailyDomant WHERE BasisDate = DATEADD(DD, -30, CAST(@dtmBasisDate AS date));
	DELETE dbo.ST_DailyCoinUse WHERE BasisDate = CAST(@dtmBasisDate AS date) AND CoinChangeCode < 224;
	DELETE dbo.ST_DailyStageEnd WHERE BasisDate = CAST(@dtmBasisDate AS date);
	DELETE dbo.ST_DailyMapStay WHERE BasisDate = CAST(@dtmBasisDate AS date);
	DELETE dbo.ST_DailyTeacher2 WHERE BasisDate = CAST(@dtmBasisDate AS date);
	DELETE dbo.ST_DailyPVP WHERE BasisDate = CAST(@dtmBasisDate AS date);
	DELETE dbo.ST_DailyPVPLadder2 WHERE BasisDate = CAST(@dtmBasisDate AS date);
	DELETE dbo.ST_DailyItemCount WHERE BasisDate = CAST(@dtmBasisDate AS date) AND ItemUseCode <> 3;

	EXEC dbo.P_DropExpiredLogTable;
	EXEC dbo.P_CreateDailyLogTable @dtmBasisDate = @dtmBasisDate;
	EXEC dbo.P_EmptyTempTables;
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
