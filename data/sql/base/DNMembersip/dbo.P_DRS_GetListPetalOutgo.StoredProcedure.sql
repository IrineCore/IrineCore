USE [dnmembership]
GO
/****** Object:  StoredProcedure [dbo].[P_DRS_GetListPetalOutgo]    Script Date: 11/14/2026 8:51:10 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[P_DRS_GetListPetalOutgo]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE [dbo].[P_DRS_GetListPetalOutgo] AS' 
END
GO
/**
version : 3
author : ???
e-mail : wgkim@eyedentitygames.com
created date : 2012-02-22

description :
	?? ?? ?? ??
result set :
	PetalAmount / int
	OutgoDate / datetime2(0)
	PetalOutgoCode / tinyint
	PetalOutgoFKey / bigint
return value :
	0 = ??? ????.
	1 = ????? Commit? ? ?? ?????. ????? Rollback???.
	100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

ALTER PROCEDURE [dbo].[P_DRS_GetListPetalOutgo]
	  @inyPetalOutgoCode tinyint = NULL --// 5=(DOORS) ???, 6=(DOORS) ?????, 7=(DOORS) ?????, 8=(DOORS) ?????
	, @inbPetalOutgoFKey bigint = NULL --// PetalOutgoCode ? ??? : 5=(DOORS) ContractID, 6=(DOORS) AuctionID, 7=(DOORS) BiddingID, 8=(DOORS) ContractID
--WITH EXECUTE AS 'SPExecutor'
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

	IF @inbPetalOutgoFKey IS NOT NULL
	BEGIN
		SELECT PetalAmount, OutgoDate, PetalOutgoCode, PetalOutgoFKey
		  FROM dbo.PetalOutgo WITH (NOLOCK, INDEX(IX_NNF_PetalOutgo_PetalOutgoFKey_PetalOutgoCode))
		 WHERE (PetalOutgoCode IN (5, 6, 7, 8))
		   AND (PetalOutgoFKey = @inbPetalOutgoFKey)
		   AND (@inyPetalOutgoCode IS NULL OR PetalOutgoCode = @inyPetalOutgoCode);
	END
	ELSE
	BEGIN
		SELECT PetalAmount, OutgoDate, PetalOutgoCode, PetalOutgoFKey
		  FROM dbo.PetalOutgo WITH (NOLOCK, INDEX(IX_NNF_PetalOutgo_PetalOutgoFKey_PetalOutgoCode))
		 WHERE (PetalOutgoCode IN (5, 6, 7, 8))
		   AND (PetalOutgoFKey IS NOT NULL)
		   AND (@inyPetalOutgoCode IS NULL OR PetalOutgoCode = @inyPetalOutgoCode);
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
