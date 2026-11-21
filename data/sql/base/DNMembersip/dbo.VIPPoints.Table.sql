USE [dnmembership]
GO
/****** Object:  Table [dbo].[VIPPoints]    Script Date: 11/14/2026 8:51:04 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[VIPPoints]') AND type in (N'U'))
BEGIN
CREATE TABLE [dbo].[VIPPoints](
	[CharacterID] [bigint] NOT NULL,
	[BasicPoint] [int] NOT NULL,
	[IncrementalPoint] [int] NOT NULL,
	[DecreasedPoint] [int] NOT NULL,
	[VIPStartDate] [smalldatetime] NOT NULL,
	[VIPEndDate] [smalldatetime] NOT NULL,
	[LastCalculatedDate] [date] NOT NULL,
	[AutoPayFlag] [bit] NOT NULL,
	[AdjustedPoint] [int] NOT NULL,
 CONSTRAINT [PK_VIPPoints] PRIMARY KEY CLUSTERED 
(
	[CharacterID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
) ON [DataFG1]
END
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[DF_VIPPoints_AutoPayFlag]') AND type = 'D')
BEGIN
ALTER TABLE [dbo].[VIPPoints] ADD  CONSTRAINT [DF_VIPPoints_AutoPayFlag]  DEFAULT ((0)) FOR [AutoPayFlag]
END
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[DF_VIPPoints_AdjustedPoint]') AND type = 'D')
BEGIN
ALTER TABLE [dbo].[VIPPoints] ADD  CONSTRAINT [DF_VIPPoints_AdjustedPoint]  DEFAULT ((0)) FOR [AdjustedPoint]
END
GO
IF NOT EXISTS (SELECT * FROM sys.foreign_keys WHERE object_id = OBJECT_ID(N'[dbo].[FK_Characters_VIPPoints_CharacterID]') AND parent_object_id = OBJECT_ID(N'[dbo].[VIPPoints]'))
ALTER TABLE [dbo].[VIPPoints]  WITH CHECK ADD  CONSTRAINT [FK_Characters_VIPPoints_CharacterID] FOREIGN KEY([CharacterID])
REFERENCES [dbo].[Characters] ([CharacterID])
GO
IF  EXISTS (SELECT * FROM sys.foreign_keys WHERE object_id = OBJECT_ID(N'[dbo].[FK_Characters_VIPPoints_CharacterID]') AND parent_object_id = OBJECT_ID(N'[dbo].[VIPPoints]'))
ALTER TABLE [dbo].[VIPPoints] CHECK CONSTRAINT [FK_Characters_VIPPoints_CharacterID]
GO
