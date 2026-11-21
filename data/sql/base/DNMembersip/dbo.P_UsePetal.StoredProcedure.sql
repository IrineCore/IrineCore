USE [dnmembership]
GO
/****** Object:  StoredProcedure [dbo].[P_UsePetal]    Script Date: 11/14/2026 8:51:10 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[P_UsePetal]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE [dbo].[P_UsePetal] AS' 
END
GO
/**
version : 3
author : ???
e-mail : ohtack@eyedentitygames.com
created date : 2013-01-21
description : ??? ?????.(??? ????? ?? ?? SP ??? ??? ????!!!) 
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
101136 = ??? ?????.
**/

ALTER PROCEDURE [dbo].[P_UsePetal]
	  @inbCharacterID bigint --// ??? ID
	, @intPetalAmount int --// ??? ?? ??
	, @inyPetalOutgoCode tinyint --// 9=???? ???
	, @inbPetalOutgoFKey bigint --// PetalOutgoCode ? ??? : 9=(DNWorld)GambleID
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
	  , @inbPetalAmount bigint
	  , @intDeductAmount int = 0
	  , @intTemp int = 0
	  , @inbPetalIncomeID bigint
	  , @intRemainPetal int
	  , @dt2Now datetime2(0) = GETDATE();

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	
	WHILE @intDeductAmount < @intPetalAmount
	BEGIN
		/**_# ?? ?? ?? ???? ????.*/
		/**_## ?? ??? ???? 101136? ?????.*/
		SELECT TOP (1) @inbPetalIncomeID = PTI.PetalIncomeID, @intRemainPetal = PTI.PetalAmount - ISNULL(V.PetalAmount, 0)
		FROM dbo.PetalIncome PTI WITH (NOLOCK)
			INNER JOIN dbo.Characters CH WITH (NOLOCK) ON PTI.AccountID = CH.AccountID
			LEFT OUTER JOIN dbo.VIX_PetalOutgoTotal V WITH (NOEXPAND, NOLOCK) ON PTI.PetalIncomeID = V.PetalIncomeID
		WHERE CH.CharacterID = @inbCharacterID AND PTI.PetalAmount > ISNULL(V.PetalAmount, 0)
		ORDER BY PTI.PetalIncomeID;

		IF @@ROWCOUNT = 0
		BEGIN
			SET @intReturnValue = 101136;
			GOTO ErrorHandler;
		END

		SET @intTemp = 
			CASE
				WHEN (@intPetalAmount - @intDeductAmount) <= @intRemainPetal THEN @intPetalAmount - @intDeductAmount
				ELSE @intRemainPetal
			END;

		SET @intDeductAmount += @intTemp;

		/**_# [PetalOutgo] ???? INSERT???.*/
		INSERT dbo.PetalOutgo (PetalIncomeID, PetalOutgoCode, PetalOutgoFKey, PetalAmount, OutgoDate)
		VALUES (@inbPetalIncomeID, @inyPetalOutgoCode, @inbPetalOutgoFKey, @intTemp, @dt2Now);
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
