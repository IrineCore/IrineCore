USE [dnmembership]
GO
/****** Object:  Table [dbo].[GiftSendScheduleTarget]    Script Date: 11/14/2026 8:51:04 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[GiftSendScheduleTarget]') AND type in (N'U'))
BEGIN
CREATE TABLE [dbo].[GiftSendScheduleTarget](
	[GiftSendScheduleID] [int] NOT NULL,
	[CharacterID] [bigint] NOT NULL,
	[DeleteFlag] [bit] NOT NULL,
 CONSTRAINT [PK_GiftSendScheduleTarget] PRIMARY KEY CLUSTERED 
(
	[CharacterID] ASC,
	[GiftSendScheduleID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
) ON [DataFG1]
END
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[DF_GiftSendScheduleTarget_DeleteFlag]') AND type = 'D')
BEGIN
ALTER TABLE [dbo].[GiftSendScheduleTarget] ADD  CONSTRAINT [DF_GiftSendScheduleTarget_DeleteFlag]  DEFAULT ((0)) FOR [DeleteFlag]
END
GO
