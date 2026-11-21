USE [dnmembership]
GO
/****** Object:  StoredProcedure [dbo].[__NX__GetBalance]    Script Date: 11/14/2026 8:51:10 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[__NX__GetBalance]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE [dbo].[__NX__GetBalance] AS' 
END
GO
-- =============================================
-- Author:		<Author,,Name>
-- Create date: <Create Date,,>
-- Description:	<Description,,>
-- =============================================
ALTER PROCEDURE [dbo].[__NX__GetBalance]
	@nvcAccountName	nvarchar(18),
	@intBalance		int OUT
WITH EXECUTE AS 'SPExecutor'
AS
BEGIN
	DECLARE @intReturnValue int = 0
	
	EXEC @intReturnValue = dbo.P_GetCashBalance
		  @intAccountID = 0
		, @nvcAccountName = @nvcAccountName
		, @intCashBalance = @intBalance OUTPUT;
		
	IF(@intReturnValue != 0)
		RETURN -1
		
	RETURN 0
END
GO
