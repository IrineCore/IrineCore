USE [dnmembership]
GO
/****** Object:  StoredProcedure [dbo].[P_PurchaseTradeItemByPetal]    Script Date: 11/14/2026 8:51:10 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[P_PurchaseTradeItemByPetal]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE [dbo].[P_PurchaseTradeItemByPetal] AS' 
END
GO
/**
version : 6
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2011-01-24

description :
	??? ??? ???? ?????.
return value :
	0 = ??? ????.
	1 = ????? Commit? ? ?? ?????. ????? Rollback???.
	100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
	101136 = ??? ?????.
history :
	2~3 = @inyPetalOutgoCode ??     by ??? at 2012-01-25
	4 = input parameter ?? (@inbPetalOutgoFKey)     by ??? at 2012-02-20
	5 = DOORS ?? ?? ??     by ??? at 2012-02-20
**/

ALTER PROCEDURE [dbo].[P_PurchaseTradeItemByPetal]
	  @inbCharacterID bigint --// ??? ID
	, @intPrice int --// ?? ??
	, @inyPetalOutgoCode tinyint = 4 --// 1=??? ??, 2=?? ???? ??, 3=??? ??, 4=??? ??? ??
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
	, @dt2Now datetime2(0) = GETDATE()
	, @inbPetalIncomeID bigint
	, @intDeductAmount int = 0
	, @intTemp int = 0
	, @intRemainPetal int;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	BEGIN TRANSACTION;

	WHILE @intDeductAmount < @intPrice
	BEGIN
		/**_# ?? ?? ?? ???? ????.*/
		/**_## ?? ??? ???? 101136? ?????.*/
		SELECT TOP (1) @inbPetalIncomeID = PTI.PetalIncomeID, @intRemainPetal = PTI.PetalAmount - ISNULL(V.PetalAmount, 0)
		FROM dbo.Characters CH WITH (NOLOCK, INDEX(PK_Characters), FORCESEEK)
			INNER JOIN dbo.PetalIncome PTI WITH (NOLOCK, INDEX(IX_NNI_PetalIncome_AccountID), FORCESEEK) ON PTI.AccountID = CH.AccountID
			LEFT OUTER JOIN dbo.VIX_PetalOutgoTotal V WITH (NOEXPAND, NOLOCK) ON PTI.PetalIncomeID = V.PetalIncomeID
		WHERE CH.CharacterID = @inbCharacterID
		  AND PTI.PetalAmount > ISNULL(V.PetalAmount, 0)
		ORDER BY PTI.PetalIncomeID
		OPTION (KEEPFIXED PLAN, FORCE ORDER);

		IF @@ROWCOUNT = 0
		BEGIN
			SET @intReturnValue = 101136;
			GOTO ErrorHandler;
		END

		SET @intTemp = 
			CASE
				WHEN (@intPrice - @intDeductAmount) <= @intRemainPetal THEN @intPrice - @intDeductAmount
				ELSE @intRemainPetal
			END;

		SET @intDeductAmount += @intTemp;

		/**_# [PetalOutgo] ???? INSERT???.*/
		INSERT dbo.PetalOutgo (PetalIncomeID, PetalOutgoCode, PurchaseOrderID, PetalAmount, OutgoDate)
		VALUES (@inbPetalIncomeID, @inyPetalOutgoCode, NULL, @intTemp, @dt2Now);
	END

	COMMIT TRANSACTION;
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
