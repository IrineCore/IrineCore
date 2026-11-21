USE [dnworld]
GO
/****** Object:  Table [dbo].[FieldDeleteLogs2]    Script Date: 11/14/2026 8:54:15 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[FieldDeleteLogs2](
	[LogDate] [datetime] NOT NULL,
	[FarmID] [int] NOT NULL,
	[FieldIndex] [smallint] NOT NULL,
	[CharacterID] [bigint] NOT NULL,
	[ItemID] [int] NOT NULL,
	[PlantDate] [datetime2](0) NOT NULL,
	[ElapsedTime] [int] NOT NULL,
	[HarvestFlag] [bit] NOT NULL
) ON [DataFG1]
GO
