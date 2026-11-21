USE [dnmembership]
GO
/****** Object:  StoredProcedure [dbo].[P_DRS_AddPetalOutgo]    Script Date: 11/14/2026 8:51:10 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[P_DRS_AddPetalOutgo]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE [dbo].[P_DRS_AddPetalOutgo] AS' 
END
GO
/**
version : 4
author : ???
e-mail : wgkim@eyedentitygames.com
created date : 2012-02-20

description :
	?? ??
return value :
	0 = ??? ????.
	1 = ????? Commit? ? ?? ?????. ????? Rollback???.
	100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
	101136 = ??? ?????.
**/
ALTER PROCEDURE [dbo].[P_DRS_AddPetalOutgo]
	  @intAccountID int --// ??? ?? ID
	, @intPetalAmount int --// ??? ?? ??
	, @inyPetalOutgoCode tinyint --// 5=(DOORS) ???, 6=(DOORS) ?????, 7=(DOORS) ?????, 8=(DOORS) ?????
	, @inbPetalOutgoFKey bigint --// PetalOutgoCode ? ??? : 5=(DOORS) ContractID, 6=(DOORS) AuctionID, 7=(DOORS) BiddingID, 8=(DOORS) ContractID
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

	IF @intPetalAmount > 0
	BEGIN
		BEGIN TRANSACTION;

		WHILE @intDeductAmount < @intPetalAmount
		BEGIN
			/**_# ?? ?? ?? ???? ????.*/
			SELECT TOP (1)
				   @inbPetalIncomeID = PTI.PetalIncomeID
				 , @intRemainPetal = PTI.PetalAmount - ISNULL(V.PetalAmount, 0)
			  FROM dbo.PetalIncome PTI WITH (NOLOCK)
				   LEFT OUTER JOIN dbo.VIX_PetalOutgoTotal V WITH (NOEXPAND, NOLOCK) ON (PTI.PetalIncomeID = V.PetalIncomeID)
			 WHERE PTI.AccountID = @intAccountID
			   AND PTI.PetalAmount > ISNULL(V.PetalAmount, 0)
			ORDER BY PTI.PetalIncomeID;

			IF @@ROWCOUNT = 0
			BEGIN
				SET @intReturnValue = 101136; -- ??? ?????.
				GOTO ErrorHandler;
			END

			SET @intTemp = CASE WHEN (@intPetalAmount - @intDeductAmount) <= @intRemainPetal THEN @intPetalAmount - @intDeductAmount ELSE @intRemainPetal END;
			SET @intDeductAmount += @intTemp;

			/**_# [PetalOutgo] ???? INSERT???.*/
			INSERT dbo.PetalOutgo (PetalIncomeID, PetalAmount, OutgoDate, PetalOutgoCode, PetalOutgoFKey)
			VALUES (@inbPetalIncomeID, @intTemp, @dt2Now, @inyPetalOutgoCode, @inbPetalOutgoFKey);
		END

		COMMIT TRANSACTION;
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
