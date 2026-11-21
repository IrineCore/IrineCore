USE [dnstaging]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListItemCountMonitoringTotal]    Script Date: 11/14/2026 9:01:06 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 2
author : ???
e-mail : wgkim@eyedentitygames
created date : 2011-03-09
description : ?? ???? ?? ???? ?? ??? ?????.
result set : BasisDate, WorldID, ItemID, ItemName, ItemLevel, ItemCount
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_GetListItemCountMonitoringTotal]
	  @datBasisDateFrom date --// ?? ?? (???)
	, @datBasisDateTo date --// ?? ?? (???)
	, @intWorldID int --// ?? ID
	, @intItemID int = NULL --// ??? ID
	, @inyItemLevel tinyint = NULL --// ??? ??
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

    /** ??? ID ? ???? ?? ???? ??? ?? ID ????? ?????. */
    IF @intItemID IS NULL
    BEGIN

        SELECT ICM.BasisDate
             , ICM.WorldID
             , ICM.ItemID
             , DI.ItemName
             , ICM.ItemLevel
             , ICM.ItemCount
          FROM dbo.ItemCountMonitoringTotal ICM WITH (NOLOCK)
               LEFT OUTER JOIN dbo.DimItems DI WITH (NOLOCK) ON (ICM.ItemID = DI.ItemID)
         WHERE (ICM.BasisDate >= @datBasisDateFrom AND ICM.BasisDate <= @datBasisDateTo)
           AND (ICM.WorldID = @intWorldID)

    END
    /** ??? ID ? ???? ???? ??? ?? ID, ??? ID, ??? ?? ???? ?????. */
    ELSE
    BEGIN

        SELECT ICM.BasisDate
             , ICM.WorldID
             , ICM.ItemID
             , DI.ItemName
             , ICM.ItemLevel
             , ICM.ItemCount
          FROM dbo.ItemCountMonitoringTotal ICM WITH (NOLOCK)
               LEFT OUTER JOIN dbo.DimItems DI WITH (NOLOCK) ON (ICM.ItemID = DI.ItemID)
         WHERE (ICM.BasisDate >= @datBasisDateFrom AND ICM.BasisDate <= @datBasisDateTo)
           AND (ICM.WorldID = @intWorldID)
           AND (ICM.ItemID = @intItemID)
           AND (@inyItemLevel IS NULL OR ICM.ItemLevel >= @inyItemLevel)

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
