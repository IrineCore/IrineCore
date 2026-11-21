USE [dnmembership]
GO
/****** Object:  Table [dbo].[TemporaryOpenedIPs]    Script Date: 11/14/2026 8:51:04 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[TemporaryOpenedIPs]') AND type in (N'U'))
BEGIN
CREATE TABLE [dbo].[TemporaryOpenedIPs](
	[TemporaryBlockID] [int] NOT NULL,
	[IP] [int] NOT NULL,
 CONSTRAINT [PK_TemporaryOpenedIPs] PRIMARY KEY CLUSTERED 
(
	[TemporaryBlockID] ASC,
	[IP] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
) ON [DataFG1]
END
GO
IF NOT EXISTS (SELECT * FROM sys.foreign_keys WHERE object_id = OBJECT_ID(N'[dbo].[FK_TemporaryBlock_TemporaryOpenedIPs_TemporaryBlockID]') AND parent_object_id = OBJECT_ID(N'[dbo].[TemporaryOpenedIPs]'))
ALTER TABLE [dbo].[TemporaryOpenedIPs]  WITH CHECK ADD  CONSTRAINT [FK_TemporaryBlock_TemporaryOpenedIPs_TemporaryBlockID] FOREIGN KEY([TemporaryBlockID])
REFERENCES [dbo].[TemporaryBlock] ([TemporaryBlockID])
GO
IF  EXISTS (SELECT * FROM sys.foreign_keys WHERE object_id = OBJECT_ID(N'[dbo].[FK_TemporaryBlock_TemporaryOpenedIPs_TemporaryBlockID]') AND parent_object_id = OBJECT_ID(N'[dbo].[TemporaryOpenedIPs]'))
ALTER TABLE [dbo].[TemporaryOpenedIPs] CHECK CONSTRAINT [FK_TemporaryBlock_TemporaryOpenedIPs_TemporaryBlockID]
GO
