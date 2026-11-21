USE [dnmembership]
GO
/****** Object:  UserDefinedFunction [dbo].[FN_IP2IPNumber]    Script Date: 11/14/2026 8:51:03 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[FN_IP2IPNumber]') AND type in (N'FN', N'IF', N'TF', N'FS', N'FT'))
BEGIN
execute dbo.sp_executesql @statement = N'/**
version : 2
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-01-04
description : IP??? ????? ?????.
**/

CREATE FUNCTION [dbo].[FN_IP2IPNumber] (
	@vchIP varchar(15)
)
RETURNS int
AS
BEGIN
	RETURN
		((CAST(PARSENAME(@vchIP, 4) AS int) - 128) * 16777216) +
		(CAST(PARSENAME(@vchIP, 3) AS int) * 65536) +
		(CAST(PARSENAME(@vchIP, 2) AS int) * 256) +
		CAST(PARSENAME(@vchIP, 1) AS int);
END
' 
END
GO
