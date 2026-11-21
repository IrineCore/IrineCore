USE [dnmembership]
GO
/****** Object:  Table [dbo].[VIPPointLogs]    Script Date: 11/14/2026 8:51:04 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[VIPPointLogs]') AND type in (N'U'))
BEGIN
CREATE TABLE [dbo].[VIPPointLogs](
	[PurchaseOrderID] [bigint] NOT NULL,
	[CharacterID] [bigint] NOT NULL,
	[BasicPoint] [int] NOT NULL,
 CONSTRAINT [PK_VIPPointLogs] PRIMARY KEY CLUSTERED 
(
	[PurchaseOrderID] ASC,
	[CharacterID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
) ON [DataFG1]
END
GO
IF NOT EXISTS (SELECT * FROM sys.foreign_keys WHERE object_id = OBJECT_ID(N'[dbo].[FK_VIPPoints_VIPPointLogs_CharacterID]') AND parent_object_id = OBJECT_ID(N'[dbo].[VIPPointLogs]'))
ALTER TABLE [dbo].[VIPPointLogs]  WITH CHECK ADD  CONSTRAINT [FK_VIPPoints_VIPPointLogs_CharacterID] FOREIGN KEY([CharacterID])
REFERENCES [dbo].[VIPPoints] ([CharacterID])
GO
IF  EXISTS (SELECT * FROM sys.foreign_keys WHERE object_id = OBJECT_ID(N'[dbo].[FK_VIPPoints_VIPPointLogs_CharacterID]') AND parent_object_id = OBJECT_ID(N'[dbo].[VIPPointLogs]'))
ALTER TABLE [dbo].[VIPPointLogs] CHECK CONSTRAINT [FK_VIPPoints_VIPPointLogs_CharacterID]
GO
