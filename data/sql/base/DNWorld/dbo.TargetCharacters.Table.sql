USE [dnworld]
GO
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[TargetCharacters](
	[CertKey] [char](32) NOT NULL,
	[RegisterDate] [datetime] NOT NULL DEFAULT (GETDATE())
)
GO
