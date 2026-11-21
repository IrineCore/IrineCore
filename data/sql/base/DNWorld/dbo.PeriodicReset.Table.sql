USE [dnworld]
GO
/****** Object:  Table [dbo].[PeriodicReset]    Script Date: 11/14/2026 8:54:15 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[PeriodicReset](
	[CharacterID] [bigint] NOT NULL,
	[PeriodicResetCode] [tinyint] NOT NULL,
	[PeriodicResetValue] [bigint] NOT NULL,
	[LastModifyDate] [smalldatetime] NOT NULL,
 CONSTRAINT [PK_PeriodicReset] PRIMARY KEY CLUSTERED 
(
	[CharacterID] ASC,
	[PeriodicResetCode] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
) ON [DataFG1]
GO
